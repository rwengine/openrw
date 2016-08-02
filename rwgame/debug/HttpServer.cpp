#include "HttpServer.hpp"
#include <engine/GameState.hpp>

#include <iostream>
#include <regex>

const char* src_debugger_js = R"(
var app = angular.module('debugApp', []);
app.controller('DebugCtrl', function($scope,$http) {
    $scope.threads = [];
    $scope.running = true;
    $scope.breakpoint = {};
    $scope.refresh = function() {
        var promise = $http.get('/state')
                       .success(function(data, status, headers, config) {
                           $scope.running = data.status == 'running';
                           $scope.threads = data.threads;
                           $scope.breakpoint = data.breakpoint;
                       });
    };
    $scope.interrupt = function() {
        var promise = $http.get('/interrupt')
                      .success(function(data, status, headers, config) {
                          $scope.refresh();
                      });
                   }
   $scope.step = function() {
       var promise = $http.get('/step')
                     .success(function(data, status, headers, config) {
                         $scope.refresh();
                     });
                  }
    $scope.continue = function() {
       var promise = $http.get('/continue')
                     .success(function(data, status, headers, config) {
                         $scope.refresh();
                     });
                   }
    $scope.refresh();
});

)";

const char* src_page = R"(
        <html ng-app="debugApp">
        <head>
            <style type="text/css">
html,body { font-family: sans-serif; }
.disassembly {
    background: white;
    border: 1px solid #CCC
}
.disassembly .breakpoint {
    background: #CFC
}
.disassembly .line-number {
    width: 6em;
    text-align: right;
    margin-right: 0.75em;
    padding-left: 0.25em;
    background: #F6F6F6;
    border-right: solid 1px #EEE;
}
            </style>
        </style>
        <body ng-controller="DebugCtrl">
        <script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.4.1/angular.min.js"></script>
        <h1>OpenRW Debugger</h1>
        <div class="debugger-controlls">
            <button ng-click="interrupt();">Interrupt</button>
            <button ng-click="step();">Step</button>
            <button  ng-click="continue();">Continue</button>
        </div>
        <span id="running" style="background: green; color: white; padding: 2px; border-radius: 2px" ng-show="running">Game is running</span>
        <div>
            {{ breakpoint.program_counter }} {{ breakpoint.thread }}
            <h2>Threads</h2>
            <ul>
                <li ng-repeat="thread in threads">
                    <h3>{{thread.name}} ({{thread.address}})</h3>
                    <i>program counter: </i> {{thread.program_counter}}<br>
                    <i>wake counter: </i> {{thread.wake_counter}} ms<br>
                    <h4>Return Stack</h4>
                    <ol>
                        <li ng-repeat="return_address in thread.call_stack">
                            Address {{return_address}}
                        </li>
                    </ol>
                    <h4>Disassembly</h4>
                    <div class="disassembly">
                        <div ng-repeat="call in thread.disassembly" class="instruction"
        ng-class="(breakpoint.program_counter == call.address && thread.address == breakpoint.thread)? 'breakpoint' : ''">
                            <span class="line-number">
                                {{ call.address }}
                            </span>
                            <span class="function">
                                {{call.function }}
                            </span>
                            <span class="parameters">
                                (
                                <span class="param" ng-repeat="param in call.arguments">
                                    {{param.value}}<span ng-show="!$last">,</span>
                                </span>
                                )
                            </span>
                        </div>
                    </div>
                    <ol>
                        <li ng-repeat="return_address in thread.call_stack">
                            Address {{return_address}}
                        </li>
                    </ol>
                </li>
            </ul>
        </div>
        <div><textarea id="state_dump"></textarea></div>
        <script src="/debugger.js"></script>
        </body>
        </html>)";

HttpServer::HttpServer(RWGame* game, GameWorld* world)
    : game(game), world(world), paused(false), lastBreakpoint(nullptr)
{}

void HttpServer::run()
{
	if (!socket.bind(8091))
		return;

	std::cout << "STARTING HTTP SERVER" << std::endl;

	TcpSocket client;
	while (game->getWindow().isOpen() && socket.listen(client)) {
		std::cout << "New connection from "
			<< client.getRemoteAddress() << ":" << client.getRemotePort()
			<< std::endl;

		std::string buffer;
		client.recv(buffer, 1024);

		std::cout << "Got " << buffer.length() << " bytes: " << buffer.c_str() << std::endl;

		try
		{
			std::regex regex_http_first_line("(\\w+)\\s+(/.*?)\\s+HTTP/\\d+.\\d+");
			std::cmatch regex_match;
			std::regex_search(buffer.c_str(), regex_match, regex_http_first_line);

			if (regex_match.size() == 3) {
				std::string http_method = regex_match.str(1);
				std::string http_path = regex_match.str(2);

				std::string response = dispatch(http_method, http_path);
				client.send(response);
			}
		}
		catch(std::regex_error er)
		{
			std::cerr << er.what() << " " << er.code() << std::endl;
		}

		client.disconnect();
	}
}

void HttpServer::handleBreakpoint(const SCMBreakpoint &bp)
{
    lastBreakpoint = &bp;
    paused = true;

    while( paused ) {
        // Do nothing
    }
}
std::string thread_stack(SCMThread& th)
{
    std::stringstream ss;
    for(unsigned int i = 0; i < th.stackDepth; ++i)
    {
        bool last = (th.stackDepth == i+1);
        ss << th.calls[i]
            << (last ? "" : ",");
    }
    return ss.str();
}

#include <script/ScriptDisassembly.hpp>

std::string describe_parameter(SCMOpcodeParameter& p)
{
    std::stringstream ss;

    switch(p.type) {
    case TGlobal:
        ss << "G: " << p.globalPtr;
        break;
    case TLocal:
        ss << "L: " << p.globalPtr;
        break;
    case TInt8:
        ss << "i8: " << std::dec << (uint16_t)((uint8_t)p.integer);
        break;
    case TInt16:
        ss << "i16: " << std::dec << (uint16_t)p.integer;
        break;
    case TInt32:
        ss << "i32: " << std::dec << (uint32_t)p.integer;
        break;
    case TString:
        ss << "str: \\\"" << p.string << "\\\"";
        break;
    case TFloat16:
        ss << "f16: " << p.real;
        break;
    default:
        ss << "OTHER";
        break;
    }

    return ss.str();
}

std::string thread_disassembly(ScriptMachine* script, SCMThread& th)
{
    std::stringstream ss;

    ScriptDisassembly ds(script->getOpcodes(), script->getFile());
    try {
        ds.disassemble(th.programCounter);
    }
    catch(SCMException& ex)
    {

    }

    auto it = ds.getInstructions().find(th.programCounter);
    for( int i = 0; i < 5 && it != ds.getInstructions().end(); ++i ) {
        bool last = i+1 == 5;
        last = last || it == --ds.getInstructions().end();
        auto opcode = it->second.opcode;
        ScriptFunctionMeta* meta = nullptr;
        ss << "{";
        if( script->getOpcodes()->findOpcode((uint16_t)opcode, &meta) )
        {
            ss << "\"address\": \"" << it->first << "\","
               << "\"function\": \"" << meta->signature << "\","
               << "\"arguments\": [";
            SCMParams& parameters = it->second.parameters;
            for(size_t p = 0; p < parameters.size(); ++p)
            {
                if(p != 0)
                    ss << ",";
                ss << "{"
                   << "\"type\": " << parameters[p].type << ","
                   << "\"value\": \"" << describe_parameter(parameters[p]) << "\""
                   << "}";
            }
            ss << "]";
        }
        else
        {
            ss << "\"err\":\"err\"";
        }
        ss << (last ? "}" : "},");
        it++;
    }

    return ss.str();
}

std::string thread(ScriptMachine* script, SCMThread& th)
{
    std::stringstream ss;
    ss << "{"
       << "\"address\": \"" << &th << "\","
       << "\"program_counter\": " << th.programCounter << ","
       << "\"name\": \"" << th.name << "\","
       << "\"wake_counter\": " << th.wakeCounter << ","
       << "\"call_stack\": [" << thread_stack(th) << "],"
       << "\"disassembly\": [" << thread_disassembly(script, th) << "]"
       << "}";
    return ss.str();
}

std::string breakpoint(const SCMBreakpoint* breakpoint)
{
    std::stringstream ss;
    ss << "{";
    if( breakpoint != nullptr )
    {
        ss << "\"program_counter\": " << breakpoint->pc << ","
           << "\"thread\": \"" << breakpoint->thread << "\"";
    }
    ss << "}";

    return ss.str();
}

std::string HttpServer::getState() const
{
    if( !paused ) {
        return R"({"status":"running"})";
    }
    else {
        std::stringstream ss;
        ss << "{";
        ss << R"("status":"interrupted",)";
        ss << R"("breakpoint": )" << breakpoint(lastBreakpoint) << ",";
        ss << R"("threads": [)";
		auto it = game->getScript()->getThreads().begin();
        for(unsigned int i = 0; i < game->getScript()->getThreads().size(); ++i,++it)
        {
            if( i != 0 )
                ss << ",";
            ss << thread(game->getScript(), *it);
        }
        ss << R"(])";
        ss << "}";
        return ss.str();
    }
}

std::string HttpServer::dispatch(const std::string& method, const std::string& path)
{
    RW_UNUSED(method);

    std::stringstream ss;
    std::string mime = "text/html";
    if(path == "/debugger.js") {
        ss << src_debugger_js;
        mime = "application/javascript";
    }
    else if(path == "/state") {
        ss << getState();
        mime = "application/json";
    }
    else if(path == "/interrupt") {
        game->getScript()->interuptNext();
        /* Block until paused is true */
        while( ! paused ) { std::this_thread::yield(); }
        ss << getState();
        mime = "application/json";
    }
    else if(path == "/step") {
        if( paused ) {
            game->getScript()->interuptNext();
            lastBreakpoint = nullptr;
            paused = false;
        }
        ss << getState();
        mime = "application/json";
    }
    else if(path == "/continue") {
        lastBreakpoint = nullptr;
        paused = false;
        ss << getState();
        mime = "application/json";
    }
    else if(path == "/") {
        ss << src_page;
    }
    else {
        ss << "HTTP/1.1 404 Not Found\n\n";
        return ss.str();
    }
    std::stringstream outs;
    outs << "HTTP/1.1 200 OK\n"
        << "Content-Type: " << mime<< "\n"
        << "Connection: Close\n"
        << "\n" << ss.str();
    return outs.str();
}

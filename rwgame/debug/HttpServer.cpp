#include "HttpServer.hpp"
#include <engine/GameState.hpp>

#include <SFML/Network.hpp>

#include <iostream>
#include <regex>

const char* src_debugger_js = R"(
var app = angular.module('debugApp', []);
app.controller('DebugCtrl', function($scope,$http) {
    $scope.threads = [];
    $scope.running = true;
    $scope.refresh = function() {
        var promise = $http.get('/state')
                       .success(function(data, status, headers, config) {
                           $scope.running = data.status == 'running';
                           $scope.threads = data.threads;
                       });
    };
    $scope.interrupt = function() {
        var promise = $http.get('/interrupt')
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
            </style>
        </style>
        <body ng-controller="DebugCtrl">
        <script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.4.1/angular.min.js"></script>
        <h1>OpenRW Debugger</h1>
        <button ng-click="interrupt();">Interrupt</button> <button  ng-click="continue();">Continue</button>
        <span id="running" style="background: green; color: white; padding: 2px; border-radius: 2px" ng-show="running">Game is running</span>
        <div>
            <h2>Threads</h2>
            <ul>
                <li ng-repeat="thread in threads">
                    <h3>{{thread.name}}</h3>
                    <i>program counter: </i> {{thread.program_counter}}<br>
                    <i>wake counter: </i> {{thread.wake_counter}} ms<br>
                </li>
            </ul>
        </div>
        <div><textarea id="state_dump"></textarea></div>
        <script src="/debugger.js"></script>
        </body>
        </html>)";

HttpServer::HttpServer(RWGame* game, GameWorld* world)
    : game(game), world(world), paused(false)
{}

void HttpServer::run()
{
	listener.listen(8091);
	listener.reuse();

	std::cout << "STARTING HTTP SERVER" << std::endl;

	while (true) {
		sf::TcpSocket client;
		if (listener.accept(client) == sf::Socket::Done) {
			std::cout << "New connection from "
				<< client.getRemoteAddress() << ":" << client.getRemotePort()
				<< std::endl;

			char buf[1024];
			size_t received;
			client.receive(buf, 1023, received);
			buf[received] = '\0';
			std::cout << "Got " << received << " bytes: " << buf << std::endl;

			std::regex regex_http_first_line("(\\w+)\\s+(/.*?)\\s+HTTP/\\d+.\\d+");
			std::cmatch regex_match;
			std::regex_search(buf, regex_match, regex_http_first_line);

			if (regex_match.size() == 3) {
				std::string http_method = regex_match.str(1);
				std::string http_path = regex_match.str(2);

				std::string response = dispatch(http_method, http_path);
				client.send(response.c_str(), response.size());
			}

			client.disconnect();
		}
	}

	listener.close();
}

void HttpServer::handleBreakpoint(const SCMBreakpoint &bp)
{
    paused = true;

    while( paused ) {
        // Do nothing
    }
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
        ss << R"("threads": [)";
        for(unsigned int i = 0; i < game->getScript()->getThreads().size(); ++i)
        {
            SCMThread& th = game->getScript()->getThreads()[i];
            bool last = (game->getScript()->getThreads().size() == i+1);
            ss << "{"
                << "\"program_counter\": " << th.programCounter << ","
                << "\"name\": \"" << th.name << "\","
                << "\"wake_counter\": " << th.wakeCounter << ""
                << (last ? "}" : "},");
        }
        ss << R"(])";
        ss << "}";
        return ss.str();
    }
}

std::string HttpServer::dispatch(std::string method, std::string path)
{
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
    else if(path == "/continue") {
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

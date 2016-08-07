import re
import lxml.etree as etree

instruction_file = "SCM.ini"
args_file = "SCM.xml"
output_functions = "functions.cpp"
output_binding = "binding.cpp"

arg_regex = "%(\d+)([-:\w]*/?%?[-_\w]*%?)"
d = etree.parse(open(args_file))

functions_file = open(output_functions, "w")
binding_file = open(output_binding, "w")

function_template = """/**
\t@brief {5}

\topcode {0:04x}{1}
*/
{2} {3} {{
\tRW_UNIMPLEMENTED_OPCODE(0x{0:04x});{4}
}}

"""

binding_template = """\tbind(0x{0:04x}, {1}, opcode_{0:04x});\n"""

type_names = {
        'ENTITY': {
            'PLAYER': 'ScriptPlayer',
            'OBJECT': 'ScriptObject',
            'CHAR': 'ScriptCharacter',
            'CAR': 'ScriptVehicle',
            'CAR_GENERATOR': 'ScriptVehicleGenerator',
            'PICKUP': 'ScriptPickup',
            'BLIP': 'ScriptBlip',
            'GARAGE': 'ScriptGarage',
            'PHONE': 'ScriptPhone',
            'SCRIPT_FIRE': 'ScriptFire',
            'SPHERE': 'ScriptSphere',
            'SOUND': 'ScriptSound',
        }
        ,
        'TYPE': {
            'INT': 'ScriptInt',
            'FLOAT': 'ScriptFloat',
            'TEXT_LABEL': 'ScriptString',
            'LABEL': 'ScriptLabel',
            'ANY': '_TODO_ANY',
        }
        ,
        'MISC': {
            'CARPEDMODEL': 'ScriptModelID',
            'PEDTYPE': 'ScriptPedType',
            'DRIVINGMODE': 'ScriptDrivingMode',
            'MISSION': 'ScriptMission',
            'PAD': 'ScriptPad',
            'BUTTON': 'ScriptButton',
            'MODEL': 'ScriptModel',
            'WEAPONTYPE': 'ScriptWeaponType',
            'THREAT': 'ScriptThreat',
            'CARLOCK': 'ScriptCarLock',
            'CARCOLOUR': 'ScriptCarColour',
            'CAMMODE': 'ScriptCamMode',
            'CHANGECAMMODE': 'ScriptChangeCamMode',
            'BLIPCOLOUR': 'ScriptBlipColour',
            'BLIP_DISPLAY': 'ScriptBlipDisplay',
            'FADE': 'ScriptFade',
            'SHADOW': 'ScriptShadow',
            'CONTACT': 'ScriptContact',
            'WEATHER': 'ScriptWeather',
            'FOLLOW_ROUTE': 'ScriptFollowRoute',
            'EXPLOSION': 'ScriptExplosion',
            'CARBOMB': 'ScriptCarBomb',
            'GANG': 'ScriptGang',
            'PEDSTAT': 'ScriptPedStat',
            'ANIM': 'ScriptAnim',
            'CORONATYPE': 'ScriptCoronaType',
            'FLARETYPE': 'ScriptFlareType',
            'POBJECT': 'ScriptPObject',
            'RADAR_SPRITE': 'ScriptRadarSprite',
            'PEDGRP': 'ScriptPedGrp',
            'CAM_ZOOM': 'ScriptCamZoom',
            'FONT': 'ScriptFont',
            'WAITSTATE': 'ScriptWaitState',
            'MOTION_BLUR': 'ScriptMotionBlur',
            'STATUS': 'ScriptStatus',
            'TIMER': 'ScriptTimer',
            'COUNTER_DISPLAY': 'ScriptCounterDisplay',
            'LEVEL': 'ScriptLevel',
            'HUD_FLASH': 'ScriptHudFlash',
            'DOOR': 'ScriptDoor',
            'RADIO': 'ScriptRadio',
            'PARTICLE': 'ScriptParticle',
            'TEMPACT': 'ScriptTempact',
            'SOUND': 'ScriptSoundType',
            'PICKUP': 'ScriptPickupType',
            'GARAGE': 'ScriptGarageType',
        }
    }

var_names = {
        'GXT Entry': 'gxtEntry',
        'X Coord': 'xCoord',
        'Y Coord': 'yCoord',
        'Z Coord': 'zCoord',
        'Player': 'player',
        'Character/ped': 'character',
        'Car/vehicle': 'vehicle',
        'Object': 'object',
        'Pickup': 'pickup',
        'Model ID': 'model',
        'Radius': 'radius',
        'X Radius': 'xRadius',
        'Y Radius': 'yRadius',
        'Z Radius': 'zRadius',
        'Time (ms)': 'time',
        #'Boolean true/false': 'boolean',
        'GXT entry': 'gxtEntry',
        'Angle': 'angle',
        'X offset': 'xOffset',
        'Y offset': 'yOffset',
        'Z offset': 'zOffset',
        'Blip': 'blip',
        'Red (0-255)': 'rColour',
        'Green (0-255)': 'gColour',
        'Blue (0-255)': 'bColour',
        'Alpha (0-255)': 'aColour',
        'Car colour ID': 'carColour',
        'Weapon ID': 'weaponID',
        'X Rotation': 'xRot',
        'Y Rotation': 'yRot',
        'Z Rotation': 'zRot',
        'Area name': 'areaName',
        'Fire': 'fire',
        'Car generator': 'carGen',
        'Blip sprite ID': 'blipSprite',
        'Ped type': 'pedType',
        '2D pixel X': 'pixelX',
        '2D pixel Y': 'pixelY',
        'Gang ID': 'gangID',
        'Explosion ID': 'explosionID',
        'Vehicle action ID': 'vehicleActionID',
        'Camera mode ID': 'cameraModeID',
        'Button ID': 'buttonID',
        'Pad ID': 'padID',
        'Weather ID': 'weatherID',
        'Sound ID': 'soundID',
        'Stat ID': 'statID',
        'Set script name': 'name',
        'CHAR': 'character',
        'GARAGE': 'garage',
        'CAR': 'vehicle',
        'PLAYER': 'player',
        'OBJECT': 'object',
        'PICKUP': 'pickup',
        'PHONE': 'phone',
        'SPHERE': 'sphere',
        'BLIP': 'blip',
        'SOUND': 'sound',
        'SCRIPT_FIRE': 'fire',
        'CAR_GENERATOR': 'carGen',
        'CARPEDMODEL': 'model',
        'PEDTYPE': 'pedType',
        'FADE': 'scriptFade',
    }

def arg_type(x, el):
    outType = ''
    outAccess = 'const '
    outRef = ''
    outName = ''
    outComment = ''
    isOut = False
    if 'Out' in el.attrib:
        outAccess = '' if el.attrib['Out'] == 'true' else 'const '
        outRef = '&'
    if 'AllowConst' in el.attrib:
        outAccess = '' if el.attrib['AllowConst'] == 'false' else 'const '
    if 'Entity' in el.attrib:
        entity = el.attrib['Entity']
        outType = type_names['ENTITY'][entity]
        if entity in var_names:
            outName = var_names[entity]
        else:
            print("Unhandled entity type: {}".format(entity))
    elif 'Enum' in el.attrib:
        enum = el.attrib['Enum']
        outType = type_names['MISC'][enum]
        if enum in var_names:
            outName = var_names[enum]
    else:
        type_ = el.attrib['Type']
        outType = type_names['TYPE'][type_]
    # Determine if this is always a local / global
    allowLocal = True
    allowGlobal = True
    if 'AllowLocalVar' in el.attrib:
        allowLocal = True if el.attrib['AllowLocalVar'] == 'true' else False
    if 'AllowGlobalVar' in el.attrib:
        allowGlobal = True if el.attrib['AllowGlobalVar'] == 'true' else False
    if allowLocal == False and allowGlobal == True:
        outRef = 'Global'
        outName = 'arg' + str(x) + 'G'
    elif allowLocal == True and allowGlobal == False:
        outRef = 'Local'
        outName = 'arg' + str(x) + 'L'
    else:
        # Can be anything, assume immediate
        pass
    if 'Desc' in el.attrib:
        outComment = el.attrib['Desc']
        if outComment == 'Boolean true/false':
            outType = "ScriptBoolean"

    return (outAccess + outType + outRef,  outName, outComment, x)

def impl_sig(opcode, sig):
    outType = 'void'
    if sig[0:2] == '  ':
        outType = 'bool'
        sig = sig.strip()
    return (outType, 'opcode_{:04x}'.format(opcode))

def adjust_args(args):
    for x in range(len(args)):
        arg = args[x]
        if len(arg[1]) == 0:
            if arg[2] in var_names:
                args[x] = (arg[0], var_names[arg[2]], arg[2], arg[3])
                arg = args[x]
            else:
                args[x] = (arg[0], 'arg' + str(arg[3]), arg[2], arg[3])
                arg = args[x]
        args[x] = arg
    return args

coordinates = ['x', 'y', 'z']
colours = ['r', 'g', 'b', 'a']

def check_vector(typeName, argName, doc, args, x):
    coords = 0
    for y in range(len(coordinates)):
        if x+y >= len(args):
            break
        if '{}{}'.format(coordinates[y], typeName) == args[x+y][1]:
            coords += 1
    if coords > 1:
        args[x] = ('ScriptVec{}'.format(coords), argName, doc, x)
        for y in range(coords-1):
            args.pop(x+1)

def check_colour(typeName, argName, doc, args, x):
    coords = 0
    for y in range(len(colours)):
        if x+y >= len(args):
            break
        if '{}{}'.format(colours[y], typeName) == args[x+y][1]:
            coords += 1
    if coords > 2:
        if coords == 4:
            args[x] = ('ScriptRGBA', argName, doc, x)
        if coords == 3:
            args[x] = ('ScriptRGB', argName, doc, x)
        for y in range(coords-1):
            args.pop(x+1)

def process_args(args):
    names = []
    x = 0
    while x < len(args):
        arg = args[x]
        if '&' not in arg[0]:
            check_vector('Coord', 'coord', 'Coordinages', args, x)
            check_vector('Rot', 'rotation', 'Rotation', args, x)
            check_vector('Offset', 'offset', 'Offset', args, x)
            check_vector('Radius', 'radius', 'Radius', args, x)
            check_colour('Colour', 'colour', 'Colour (0-255)', args, x)
        x += 1
    return args

def finalize_args(args):
    names = []
    # Accumulate counts
    name_counts = {}
    for arg in args:
        name_counts[arg[1]] = (name_counts[arg[1]] if arg[1] in name_counts else 0) + 1
    for x in range(len(args)):
        arg = args[x]
        orgname = arg[1]
        if name_counts[orgname] > 1:
            args[x] = (arg[0], arg[1] + str(names.count(orgname)), arg[2], arg[3])
            names.append(orgname)
    return args


with open(instruction_file) as f:
    for line in f:
        m = re.match("([0-9A-Za-z]+)=(-?\d+),(.*)$", line)
        if m is not None:
            opcode = int(m.group(1), 16)
            argc = int(m.group(2))
            func = m.group(3)
            am = re.findall(arg_regex, line)
            sig = impl_sig(opcode, func)
            xpath = "//Command[@ID=\"0x{:x}\"]".format(opcode)
            res = d.xpath(xpath)
            args = []
            if len(res) != 0:
                res = res[0]
                sa_name = res.attrib['Name']
                if sig != sa_name and False:
                    print("Name mismatch: {} v {}".format(sig, sa_name))
                eargs = res.xpath("Args/Arg")
                for x in range(len(eargs)):
                    args.append(arg_type(x+1, eargs[x]))
            args = adjust_args(args)
            args = process_args(args)
            args = finalize_args(args)
            postfix = ''
            if argc >= 0:
                signame = sig[1] + "(const ScriptArguments& args" + ''.join([", {} {}".format(x[0], x[1]) for x in args]) + ")"
                for a in args:
                    postfix += "\n\tRW_UNUSED({});".format(a[1])
            else:
                signame = sig[1] + "(const ScriptArguments& args)"
            postfix += "\n\tRW_UNUSED(args);"
            argsdoc = ""
            if len(args) > 0:
                argsdoc = "\n" + "\n".join(["\t@arg {} {}".format(x[1], x[2]) for x in args])
            if sig[0] == 'bool':
                postfix += '\n\treturn false;'
            functions_file.write(function_template.format(opcode, argsdoc, sig[0], signame, postfix, func))
            parameters = ['args.getAs<{}>()'.format(type_) for type_, _, _, _ in args]
            binding_file.write(binding_template.format(opcode, argc, ", ".join(parameters)))
            if len(am) != abs(argc) and False:
                raise RuntimeError("Mismatched argument count: {} v {}".format(len(am), argc))


import argparse
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser(description='Run real BaserowMCU source with a fake HTTP transport and real ArduinoJson 7.')
parser.add_argument('--compiler', default='clang++', help='C++ compiler path (clang++ or g++); Zig is also supported')
parser.add_argument('--zig', action='store_true', help='Append c++ after the compiler executable')
parser.add_argument('--arduinojson', required=True, help='ArduinoJson src directory')
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='baserow-test-') as build:
    for platform in ('ESP32', 'ESP8266'):
        output = pathlib.Path(build) / (platform + '.exe')
        command = [args.compiler] + (['c++'] if args.zig else []) + [
            '-std=c++17', '-D' + platform,
            '-DARDUINOJSON_ENABLE_ARDUINO_STRING=1',
            '-DARDUINOJSON_ENABLE_ARDUINO_STREAM=0',
            '-DARDUINOJSON_ENABLE_ARDUINO_PRINT=0',
            '-DARDUINOJSON_ENABLE_PROGMEM=0',
            '-I' + str(root / 'extras' / 'tests' / 'mocks'),
            '-I' + str(root),
            '-I' + args.arduinojson,
            str(root / 'extras' / 'tests' / 'behavior.cpp'),
            str(root / 'BaserowMCU.cpp'),
            '-o', str(output)]
        subprocess.run(command, check=True)
        subprocess.run([str(output)], check=True)

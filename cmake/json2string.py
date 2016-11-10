import argparse
import json

GENERATE_HEADER = '/* This file was auto-generated. Do not modify! */ \n'

def main():
    parser = argparse.ArgumentParser(description='Converting JSON file into std::string')

    parser.add_argument('-i', '--input', help = 'Input JSON file', required = True )
    parser.add_argument('-o', '--output', help = 'Output JSON file', required = True )
    args = parser.parse_args()

    #Open and parse JSON file
    fp = open(args.input, 'r')
    json_object = json.loads(fp.read())
    fp.close()

    #Save JSON to file
    fp = open(args.output, 'w')
    fp.write(GENERATE_HEADER)
    fp.write('std::string val = "%s";' % json.dumps(json_object).replace("\"", "\\\""))
    fp.close()

if __name__ == '__main__':
    main()

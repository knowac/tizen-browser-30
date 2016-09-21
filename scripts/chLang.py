# Translation script
# Reads files recursive from given directory
# replaces all found strings with translation by
# key, which is read from en.po file.
# Strings which have no translation available in en.po
# are left unchanged.

import os
import sys
import getopt

texts = { }
filterKeywords = ['BROWSER_LOG', '#include', 'EXPORT_SERVICE', 'elm_object_part_content', \
    'evas_object_smart_callback', 'elm_object_style', 'elm_layout_file', '.png', 'elm_object_item_part_content', \
    'IDS_', 'bp_', 'elm_object_signal_emit', '.cpp', '.edj', 'item_style', 'edje_object_signal_callback', \
    'evas_object_del', '_']
verbose = False

def translateLine(line):
    translated = False
    splited = line.split('"')
    if len(splited) > 1:
        for i in range(len(splited) / 2):
            strNo = i * 2 + 1       # odd numbers
            # string longer than 1, check if there is translation for string
            if len(splited[strNo]) > 1 and splited[strNo] in texts:
                print splited[strNo] + ' \t ===> \t ' + texts[splited[strNo]]
                splited[strNo] = '_("' + texts[splited[strNo]] + '")'
                translated = True
            else:
                if verbose:
                    print 'not found: "' + splited[strNo] + '"'
                splited[strNo] = '"' + splited[strNo] + '"'

    if translated:
        return "".join(splited)
    else:
        return ""

def takeStringBetweenQuotations(s):
    return s.split('"')[1::2][0]

def filterCheck(line):
    for keyword in filterKeywords:
        if keyword in line:
            return True
    return False

def translateFile(f):
    foutData = []
    fileTranslated = False
    with open(f, 'r') as fin:
        for line in fin:
            lineTranslated = False
            if not filterCheck(line):
                newLine = translateLine(line)
                if len(newLine) > 0:
                    fileTranslated = True
                    foutData.append(newLine)
                else:
                    foutData.append(line)
            else:
                foutData.append(line)
    # Write the file out again
    if fileTranslated:
        print f
        print '\n'
        with open(f, 'w') as fout:
            fout.write("".join(foutData))
    elif verbose:
        print f
        print '\n'

def readKeys(poFile):
    with open(poFile,'r') as lang:
        value = ''
        for line in lang:
            if (line.startswith('msgid')):
                value = takeStringBetweenQuotations(line)
            elif (line.startswith('msgstr')):
                key = takeStringBetweenQuotations(line)
                if not value: raise Exception('Unknown file format, msgid should be read before msgstr')
                texts[key] = value

def translateFilesRecursive(directory):
    print 'Translated files:'
    # replace strings in all files
    for root, subFolders, files in os.walk(directory):
        #for directory in subFolders:
        for f in files:
            if f.endswith('.cpp'):
                translateFile(os.path.join(root, f))

if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "vh")

    for o, a in opts:
        if o == "-v":
            verbose = True
        elif o == "-h":
            args = []
        else:
            assert False, "unhandled option"

    if len(args) == 0:
        print 'usage: python ' + os.path.basename(__file__) + ' ./location/of/en.po ./directory/to/translate '
        print 'e.g.: python ' + os.path.basename(__file__) + ' ../res/locale/en.po ../services '
        print 'add -v for verbose (untranslated strings will be printed)'
    else:
        readKeys(args[0])
        translateFilesRecursive(args[1])

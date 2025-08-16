import argparse
import os
import json

keys = {}
langs = set()

def generateHeader(pathHeader_):
    global keys, langs
    with open(pathHeader_, 'w+', encoding='utf-8') as f:
        f.write(
"""#ifndef LOCALIZATION_GEN_H_
#define LOCALIZATION_GEN_H_
#include <string>
#include <array>

#define GENERATE_LOCALIZED_KEY(key, id) \\
    static constexpr std::string key() \\
    { \\
        return (*m_currentStrings)[id]; \\
    }

struct ll
{
public:
    static void setLang(const std::string &lang_);
    static void load();

""")
        idx = 0
        for key in keys:
            f.write("    GENERATE_LOCALIZED_KEY(" + key + ", " + str(idx) + ")\n")
            idx += 1
        
        f.write(
"""
private:
""")
        f.write("    using LocalMap = std::array<std::string, " + str(len(keys)) + ">;\n")
        for lang in langs:
            f.write("    static LocalMap m_strings" + lang + ";\n")

        f.write(
"""    static LocalMap *m_currentStrings;
};

#endif
""")
        
def generateCpp(pathCpp_):
    global keys, langs
    with open(pathCpp_, 'w+', encoding='utf-8') as f:
        f.write(
"""#include "LocalizationGen.h"
#include "FilesystemUtils.h"
#include <nlohmann/json.hpp>
#include <fstream>

""")
        for lang in langs:
            f.write("ll::LocalMap ll::m_strings" + lang + ";\n")
        f.write(
"""ll::LocalMap *ll::m_currentStrings = &ll::m_stringsen;

void ll::setLang(const std::string &lang_)
{
""")
        firstLang = True
        for lang in langs:
            if lang != "en":
                f.write("    ")
                if (not firstLang):
                    f.write("else ")
                f.write('if (lang_ == "' + lang + '")\n')
                f.write("        m_currentStrings = &m_strings" + lang + ";\n")
                firstLang = False
        f.write(
"""    else
        m_currentStrings = &m_stringsen;
}

void ll::load()
{
""")
        for lang in langs:
            f.write(
"""    {
        std::ifstream jsonIn(Filesystem::getRootDirectory() + "Localization/""" + lang + """/strings.json");
        auto jsonData = nlohmann::json::parse(jsonIn);
""")
            idx = 0
            for key in keys:
                f.write('        m_strings' + lang + "[" + str(idx) + '] = jsonData["' + key + '\"];\n')
                idx += 1
            f.write("    }\n")

        f.write(
"""}
""")
            

def handle_json(jsonPath_, lang_):
    print("Handling json at \"" + jsonPath_ + "\" for " + lang_)
    global keys
    with open(jsonPath_) as f:
        data = json.load(f)
        for key in data.keys():
            if key in keys:
                keys[key].add(lang_)
            else:
                keys[key] = set()
                keys[key].add(lang_)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--header', required=True)
    parser.add_argument('--cpp', required=True)
    parser.add_argument('--jsonroot', required=True)
    args = parser.parse_args()
    print(args)

    jsondir = os.fsencode(args.jsonroot)
    for file in os.scandir(jsondir):
        if file.is_dir():
            lang = file.name.decode("utf-8")
            langs.add(lang)
            print("Found lang: ", lang)
            stringsfile = file.path.decode("utf-8") + "/strings.json"
            if os.path.exists(stringsfile) and os.path.isfile(stringsfile):
                handle_json(stringsfile, lang)
            else:
                print("WARNING: no strings.json for lang \"" + lang + "\"")

    print("Found keys: ", keys)
    print("Found langs: ", langs)

    noError = True

    if "en" not in langs:
        errLine = "ERROR: default english localization (en) was not provided"
        print(errLine)
        with open(args.header, 'w+', encoding='utf-8') as f:
            f.write("#error " + errLine + "\n")
            noError = False

    for (k, v) in keys.items():
        if v != langs:
            errLine = "ERROR: key " + k + " has languages " + str(v) + ", but expected " + str(langs)
            print(errLine)
            with open(args.header, 'w+', encoding='utf-8') as f:
                f.write("#error " + errLine + "\n")
                noError = False
    
    if noError:
        generateHeader(args.header)
        generateCpp(args.cpp)
        
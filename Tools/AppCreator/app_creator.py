import argparse
import os
import shutil

# FactoryData_switch
_app_path = os.path.join(os.path.dirname(__file__), "..", "..", "Applications", "Matter")
_ref_app_path = os.path.abspath(os.path.join(_app_path, "base"))

list_to_ignore = list()
list_to_ignore.append("basename")

# A list of reference files that need updated content with the new app_name
files_to_update= list()
files_to_update.append(os.path.join(_ref_app_path,"README.md"))
files_to_update.append(os.path.join(_ref_app_path,"Makefile.base_qpg6105_release"))
files_to_update.append(os.path.join(_ref_app_path,"Makefile.base_qpg6105"))
files_to_update.append(os.path.join(_ref_app_path,"base_qpg6105_postbuild.sh"))
files_to_update.append(os.path.join(_ref_app_path,"base_qpg6105_release_postbuild.sh"))
files_to_update.append(os.path.join(_ref_app_path,"include","AppTask.h"))
files_to_update.append(os.path.join(_ref_app_path,"gen","base_qpg6105", "qorvo_internals.h"))
files_to_update.append(os.path.join(_ref_app_path,"gen","base_qpg6105", "qorvo_config.h"))
files_to_update.append(os.path.join(_ref_app_path,"gen","base_qpg6105_release", "qorvo_internals.h"))
files_to_update.append(os.path.join(_ref_app_path,"gen","base_qpg6105_release", "qorvo_config.h"))
files_to_update.append(os.path.join(_ref_app_path, "..", "..", "..", "Libraries", "Qorvo", "FactoryData", "Makefile.FactoryData_example"))
files_to_update.append(os.path.join(_ref_app_path, "..", "..", "..", "Libraries", "ThirdParty", "Matter", "Makefile.Matter_base_qpg6105"))
files_to_update.append(os.path.join(_ref_app_path, "..", "..", "..", "Tools", "FactoryData", "Credentials", "example.factory_data_config"))

'''
Copy the application contents of the Base application to the new folder structure
'''
def copy_reference_app_tree(target_app_name):
    _tgt_app_path = os.path.abspath(os.path.join(_ref_app_path, "..", target_app_name))
    # copy the Matter application layer
    print("Copying %s -> %s" %(_ref_app_path, _tgt_app_path))
    shutil.copytree(_ref_app_path, _tgt_app_path)

    # -- update all de directory names
    # Directories are moved first before files to avoid duplicates and
    # missing directories.
    root_target_filepath = _ref_app_path.replace("base", target_app_name, 1)
    for dirname, dirs, files in os.walk(root_target_filepath):
        # print("dn:%s, d:%s, f:%s" %(dirname, dirs,files))
        for directoryname in dirs:
            if "base" not in directoryname:
                continue
            ref_path = os.path.abspath(os.path.join(dirname, directoryname))
            tgt_path = ref_path.replace("base", target_app_name)

            if not os.path.isdir(ref_path):
                raise Exception("Dir %s doesn't exist" % filepath)
            print("mv %s -> %s" %(ref_path, tgt_path))
            shutil.move(ref_path, tgt_path)

    # -- update all de file names
    for dirname, dirs, files in os.walk(root_target_filepath):
        # print("dn:%s, d:%s, f:%s" %(dirname, dirs,files))
        for filename in files:
            if "base" not in filename:
                continue
            ref_path = os.path.abspath(os.path.join(dirname, filename))
            tgt_path = ref_path.replace("base", target_app_name)

            if not os.path.isfile(ref_path):
                raise Exception("File %s doesn't exist" % filepath)

            print("mv %s -> %s" %(ref_path, tgt_path))
            shutil.move(ref_path, tgt_path)

    # create new instance of the files to update
    for filepath in files_to_update:

        # replace base / example in original file with new app_name
        tgt_filepath = filepath.replace("base", target_app_name)
        file_to_update_in_target_folder = tgt_filepath.replace("example", target_app_name)

        print("Copying %s -> %s" %(filepath, file_to_update_in_target_folder))
        shutil.copy(filepath, file_to_update_in_target_folder)

'''
Update all files with new name
'''
def perform_file_updates(files_to_update, target_app_name):
    # -- update the file contents
    for filepath in files_to_update:
        
        # replace base / example in original file with new app_name
        target_filepath = filepath.replace("base", target_app_name)
        target_filepath = target_filepath.replace("example", target_app_name)
        print("updating: %s" % target_filepath)

        # replace all references of "base" the file with the "<target_app_name>"
        with open(filepath) as f:
            s = f.read()
            for name in list_to_ignore:
                placeholdername = name.replace("base", "__placeholder__")
                s = s.replace(name, placeholdername)
                s = s.replace("base", target_app_name)
                s = s.replace("FactoryData_example", "FactoryData_" + target_app_name)
                s = s.replace("Base-Matter-app", target_app_name)
                s = s.replace("Qorvo Matter example", "Qorvo Matter " + target_app_name)
                s = s.replace("example.factory_data_config", target_app_name + ".factory_data_config")
                s = s.replace(placeholdername , name)

        with open(target_filepath, "w") as f:
            f.write(s)

'''
Validate if the appname is acceptable
'''
def validate_appname(target_app_name):
    _tgt_app_path = os.path.abspath(os.path.join(_ref_app_path, "..", target_app_name))

    # validate if app path already exists
    if os.path.isdir(_tgt_app_path):
        raise Exception("Can't create new app because %s already exists" %(_tgt_app_path))


def main():
    parser = argparse.ArgumentParser(description='Create new application from Base example application')
    parser.add_argument('--app-name', required=True, metavar='<appName>', dest='app_name', type=str, help='Name of the new application')

    args = parser.parse_args()

    app_name = args.app_name

    validate_appname(app_name)

    copy_reference_app_tree(app_name)

    perform_file_updates(files_to_update, app_name)

    return

if __name__ == "__main__":
    main()

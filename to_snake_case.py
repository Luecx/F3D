import os
import re
import argparse

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def build_rename_map(root_dir):
    rename_map = {}
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            new_filename = camel_to_snake(filename)
            if filename != new_filename:
                rename_map[os.path.join(dirpath, filename)] = os.path.join(dirpath, new_filename)
    return rename_map

def replace_references(root_dir, rename_map):
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            with open(file_path, 'r') as file:
                content = file.read()

            for old_name, new_name in rename_map.items():
                old_base = os.path.basename(old_name)
                new_base = os.path.basename(new_name)
                content = content.replace(old_base, new_base)

            with open(file_path, 'w') as file:
                file.write(content)

def rename_files(rename_map):
    for old_name, new_name in rename_map.items():
        os.rename(old_name, new_name)

def main(root_dir):
    rename_map = build_rename_map(root_dir)
    replace_references(root_dir, rename_map)
    rename_files(rename_map)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Recursively rename files from camelCase to snake_case and update references.')
    parser.add_argument('directory', type=str, help='The root directory to start renaming files.')

    args = parser.parse_args()
    main(args.directory)

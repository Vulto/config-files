# NOPEN
MimeLauncher, also known as nopen, is a minimalistic utility designed to launch files based on their MIME types. The program utilizes the magic library to identify the MIME type of a given file and then matches it with a predefined application in a user-customizable configuration file.

## Requirements
C compiler
magic library

## Installation
```
git clone https://github.com/Vulto/nopen.git
cd nopen

./c -c (Compiles) -l (Link) -i (install)
./c -cli at once is what you need to make it compile, link and install
```
## Usage
```
./nopen <filename>
```

## Configuration
Customize the config.h file to define file type mappings. Add entries for MIME types and their associated applications:
// main.c
```
const FileTypeMapping fileTypes[] = {
    { "application/pdf", "evince" },
    // Add more mappings as needed
};
```

### Contributing
Contribute to nopen by opening issues or submitting pull requests. Your feedback and enhancements are highly appreciated!

#### License
This project is licensed under the MIT License - see the LICENSE file for details.

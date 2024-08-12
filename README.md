# Shell-Implementation
Creating a shell with some advanced features
Certainly! Here's a comprehensive README file for your shell program:

## Overview

**5air_Shell** is a simple command-line shell implementation in C that supports built-in commands, external commands, input/output redirection, and piping. It is designed to demonstrate basic shell functionalities and can be used as a starting point for building more advanced shell features.

## Features

- **Built-in Commands**:
  - `currDir`: Print the current working directory.
  - `Echo <string>`: Print the user input string to stdout.
  - `cop <sourcePath> <destinationPath> [-a]`: Copy a file to another location. Use the `-a` option to append to the target file.
  - `MV <sourcePath> <destinationPath>`: Move a file to another location.
  - `mycd <path>`: Change the current working directory to the specified path.
  - `Help`: Print the help message.
  - `Exit`: Print 'Good Bye' and terminate the shell.
  - `Free`: Print RAM and swap area information.
  - `Uptime`: Print system uptime and idle time.
  - `type <command>`: Check if a command is a built-in command or an executable.

- **External Commands**: Execute external commands with support for input/output redirection and piping.

## Compilation

To compile the shell program, use the `gcc` compiler. Ensure that both `main.c` and `commands.c` are present in the same directory.

```bash
gcc main.c commands.c -o ebashell
```

## Usage

1. **Run the Shell**:

   Execute the compiled shell program:

   ```bash
   ./ebashell
   ```

2. **Shell Prompt**:

   The shell prompt will display as `5air>`. You can enter commands and interact with the shell.

3. **Built-in Commands**:

   - To execute a built-in command, simply type the command followed by any required arguments. For example:
     ```bash
     currDir
     Echo Hello, World!
     cop source.txt destination.txt -a
     ```

4. **External Commands**:

   - You can run external commands similarly to how you would in a typical Unix-like shell. For example:
     ```bash
     ls -l
     ```

5. **Redirection**:

   - Redirect input and output using `>`, `<`, `>>`, and `2>`. For example:
     ```bash
     ls > output.txt
     cat < input.txt
     ls >> output.txt
     ls 2> error.log
     ```

6. **Piping**:

   - Use the pipe `|` operator to chain commands. For example:
     ```bash
     ls | grep ".c"
     ```

7. **Help Command**:

   - To display the list of supported commands and their usage, type:
     ```bash
     Help
     ```

## Example Session

```bash
$ ./ebashell
5air> currDir
The current working directory is: /home/user/Desktop/eShellTask
5air> Echo Hello, eShell!
Hello, eShell!
5air> ls | grep "eShell"
commands.c
commands.h
main.c
5air> ls > output.txt
5air> cat output.txt
commands.c
commands.h
main.c
5air> Exit
Good Bye :)
```

## Troubleshooting

- **`execvp: No such file or directory`**:
  - This error occurs when attempting to execute a command that is not found. Verify the command or check if the command is correctly installed.

- **Circular Redirection Error**:
  - If you attempt to redirect input and output to the same file, you will receive an error. Ensure that input and output files are distinct.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Contributing

Contributions are welcome! If you have suggestions or improvements, please submit a pull request or open an issue.

## Contact

WhatsApp: +201021811895
baseldawoud2003@gmail.com

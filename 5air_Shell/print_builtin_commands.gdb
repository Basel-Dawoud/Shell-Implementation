# Define a custom command to print the built-in commands
define builtincmd
    set $i = 0
    while (builtinCommands[$i] != 0)
        printf "builtinCommands[%d] = %s\n", $i, builtinCommands[$i]
        set $i = $i + 1
    end
end

# Break at main to ensure the program is running
break main
run


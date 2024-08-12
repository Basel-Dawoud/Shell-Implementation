# Define a custom command to print the built-in commands
define builtincmd
    # Print the base address of the array
    print builtinCommands
    
    # Define the number of elements in the array
    set $i = 0
    
    # Loop through the array and print each element
    while (builtinCommands[$i] != 0)
        printf "builtinCommands[%d] = %s\n", $i, builtinCommands[$i]
        set $i = $i + 1
    end
end

# Break at main to inspect before running
break main

# Run the program
run


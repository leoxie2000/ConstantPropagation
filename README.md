# Constant Propagation (Yi Xie, 05/22/2022)

## Design

The design of this pass lies in the use of the use list. It doesn't require any other passes to work along with it. When the pass is called, it loops through the list of instructions, add them to a working set, and see if there's any instruction that needs to be folded based on its opcode, either evaluating to binary, unary, or comparison. 

After evaluating what those instructions need to be folded into, we look at the use-list of those instructions, and add them to the working set as well. Via this process, we continue to add/pop from the working set until its empty. 

For this design it's hard to evaluate how many passes we have gone through, since through every iteration it continues to update the list and add things to it until nothing changes.

## Using the code

For this code, simply use `make` to compile, and then use `./optimize.sh [filename.ll]`  to run the script on targeted ll file. However, if run for the first time, one has to run `chmod +x optimize.sh` first to give it access.

class Mutator:


'''

    Mutator for mov instructions which store a value in a register

    For instance, mov eax, 1000 becomes

    xor eax, eax
    add eax, 23
    sub eax, <someval>

    This works by choosing a random instruction to zero the register and then
    it picks a random number of instructions to generate which the end result
    is the original value of the mov instruction being stored in the file
    
'''

class MovImmdMutator(Mutator):

    

'''
    An object file has many functions and a function has many instructions
'''

class ObjectFile:
class CoffObjectFile:

'''

    Fix relocations we need to fix relocations in binary and keep track of changes
    Fix relative jumps (we need to fix relative jumps so that they go to the correct location

'''

class Function:
class Instruction: 

'''

    Choose random junk/zero instruction from database of all possible
    combinations of instructions

'''

class GenZeroInstruction:
class GenJunkInstruction:

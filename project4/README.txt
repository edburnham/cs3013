Edward Burnham
Michael Caldwell

README for Project 4

The purpose of this project is to implement a memory management unit (MMU) that handles paging for at most 4 processes. The program vmem (short for virtual memory) accepts three commands: map, load, and store. To execute the commands, they are typed nto the command line in this form: 

process_id,instruction_type,virtual_address,value

Process_ids take on values from 0 - 3, instruction_type can be either of the three instrucions, virtual_address is the virtual address the user wishes to load or store from or to, and value is the read and write permissions, which is set when a page is mapped on a map instruction. In addition, a Round Robin algorithm chooses which page to evict during swapping and swapping can be done on all instructions.

When the map instruction is given by the user, vmem allocates a physical page for the given PID and a page table that holds inforation about the processes pages such as, PFN, read/write permissions, PID, and VPN. map either creates a page table and a page when the PID is first seen by vmem, or just a page for an already existing PID. map checks whether there is room for a new page in memory, if there is not, two pages are swapped out and a new page and page table are inserted (possibly taken from hardDisk.txt if exists there) in memory for the new or old PIDs process.

When the store instruction is given, the value is stored at the virtual_address and the physical address which is in the page in physical memory for that PID that called store. If the page is not in memory a page is evicted and the needed page is swapped in from the hard disk.

When the load instruction is given, the value at virtual_address is dispayed to the user. The load function determines if the desired page is in memory, if it is not, it is swapped in, but another oage is swapped out.

Tetsing:


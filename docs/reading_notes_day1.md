#Key Insights from Ulrich Drepper's Paper "What Every Programmer Should Know About Memory"

## Section 2.1 - Why Caches Exist (RAM and why its slow)
- 2 types of RAM - Static (SRAM) and Dynamic (DRAM)
- Static RAM
    - 6 transistors per cell 4 transistor ones exist but have disadvantages
    - 2 transistors are connected to BL and BL' (one each)
    - other 4 transistors form 2 cross-coupled invertors
    - 2 stable states 0 and 1 which are stable as long as power is on
        - no refresh cycles needed
    - WL (word access line) is raised to access contents of the cell
        - access is done on BL and BL'
    - To overwrite the state of the cell 
        - BL and BL' are set to values before WL is raised
        - works because outside drivers are more powerful than the 4 transistors
          hence they are overwritten
    - Uses a rectangular signal, easier to read binary data with sharp changes 

- Dynamic RAM
    - 1 transistor (M) and 1 capacitor (C), much simpler than static RAM cell
    - stores state in capacitor
    - transistor M gaurds access to the state stored in C
    - AL (access line) is raised to read the state of the cell
        - this causes current to flow or not on DL depending on the state stored
    - To write to the cell we set value on DL and raise AL
        - AL has to be raised for long enough to charge or drain the capacitor

    - Problems with DRAM
        - Since we use a capacitor, reading discharges it.
        - capacity of capacitor is usually low to accomodate huge number of cells
            - femto farad range or lower
            - which means it takes only a short amount of time to discharge
            - this is called "leakage"
        - This is is why cells must be recharged frequently (usually every 64ms)
            - No access to the memory possible during refresh cycle
            - this stall can be upto 50% memory accesses
        - Information read is not directly useful due to the small amount of charge
            - a sense amplifier needs to be connected to distinguish between 0 and 1
        - Reading causes cahrge to deplete
            - every read needs to be followed by refresh
            - done automatically by feeding sense amplifier data charge back to the cell
        - Charging and draining a capacitor is not instant
            - the signals recieved by sense amplifier are not rectangular
            - a conservative estimate must be made as to when the output is usable or not
            - delay in time taken to read data unlike SRAM

    - Advantages of DRAM
        - since only 1 capacitor and 1 transistor per cell saves a lot of space.
        - structure is also more regular so they can be packed tightly
        - huge cost savings which makes it more favourable than SRAM

- DRAM access
    - Program provides virtual memory address
    - processor translates into a physical address 
    - memory controller selects teh RAM chip corresponding to the physical address
    - Since each address location would need to have a huge number of address lines 
        - each address line is chosen based on some smaller set of address lines
        - a demultiplexer is used to choose the binary number that represents the address
        - N lines can give 2^N output lines
    - This direct approach using a demux with N lines works fine for small chips
    - Size of demux grows exponentially with number of address lines
        - 30 address lines would need 2^30 select lines
        - size and time for demuxing would also increase
        - transmitting signals on each of these lines aslo becomes complicated
    - Cells are organized into rows and columns
        - with this arrangement one demux and one mux of half the size is sufficient
        - Row and Column address selection is used using (RAS and CAS)
            - Whole Rows address is sent using the RAS demultiplexer
            - The particular Column is selected by the CAS multiplexer while reading
            - based on that value the column is made available to the DATA pin
            - multiple of these are done in parallel till the DATA bus width worth of data
              is produced.
        - While writing
            - opposite is done where value is set on the data bus 
            - RAS' and CAS' are used to select the cell where the content is written to.
    - Timing of how long a signal must be put onto the data bus, amplification of signals
      delay before data is availble for reading complicates the design.
    - Using 30 pins per RAM module (collection of RAM chips) for parallel reads is not 
      feasible.
        - for this reason address is transferred in 2 parts
        - first part to select the row which remains active until revoked
        - second part to select the column.
        - few more lines are  needed to indicate if RAS and CAS are available
        - this saves half the number of address lines needed.
        - has its own issues

- Conclusion
 - SRAM is larger and more expensive than DRAM but faster.
 - cells need to be selected induvidually to be used.
 - number of address lines needed is important to cost
 - DRAM reads and write have latency due to capacitor recharge and discharge

-------------------------------------------------------------------------------------------    
## Section 3.1 - CPU Caches
- early archs had one level of caching which was directly connected to the CPU
    - the bus connects the main memory to the cache and the CPU is connected to the cache
    - essentially a write through caching mechanism
    - fast connection between the CPU and cache
    - most architectures us Von Nuemann these days but seperate caches for data and code
        - icache for instructions and data cache
        - done by intel since 1993
        - this is done because memory regions for code and data are seperate
        - further solidified because of instruction decoding step in the pipeline which is           slow and can benifit from decoded instructions cache.
- Once speed of cache was too fast another level of cache was added
    - slower and bigger than first layer
    - these days even 3 layers of caches are common
- L1i and L1d cache, L2 cache and L3 cache
    - data doesnt have to write through the higher level caches before going to main memory

- Multi processor system
    - processors are an entire unit consisting of cores and thread and connect to 
      main memory directly via bus
    - multi core have seperate copies of all required hardware resources 
      and run indpendently
        - have their own L1 cache, usually share L2 cache and L3 cache
    - threads only have seperate registers (sometimes even that is shared)
        - threads share L1 cache

-------------------------------------------------------------------------------------------

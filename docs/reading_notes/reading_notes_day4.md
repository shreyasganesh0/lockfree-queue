# Hazard Pointers

## Hazard Pointers: Safe Memory Reclamation for Lock Free Objects

### Intro
- A shared object is lock free - if a thread performs some finite number
  of operations on the object then some thread must have made progress 
  towards completing an operation on the object
- Biggest concern about lock free algorithms is reclaiming memory
    - with a lock you can block other threads from accessing the object
      if when you remove the memory of some object
    - not as easy when there is no lock to garuntee that memory
      cant be accessed casuing use-after-free
- in order for the object to be lock free all threads must have 
  unobstructed access to the object
    - this can cause memeory corruption if the space is reclaimed
      by one thread and accesses by another
- Inefficient or unfeasible methods
    - IBM tag method that requires double width instructions not availble on
      64 bit processors
    - Lock free reference counting that requires unavaible strong 
      multiaddress atomic primitives
    - aggregate reference counting or per thread timestamps that need
      special scheduler support to be non blocking

- Hazard pointers 
    - Benifits
        - constant amortized time per retired node
        - upper bounds retired nodes not eligible for use at a given point
          regardless of failures
        - wait free without weaking progress garuntee
        - does not need special instructions or kernel support
### Implementation
- one or two single writer -multi reader shared pointers (hazard pointers)
- associated with each thread that intends to access lock free objects
- hazard pointer is either null or points to an object that may be used
  by the thread without validation
- only written by owner thread and read by other threads
- uses lock free algorithmns to garuntee no thread access a node when
  it possible that the object is removed unless at least one of the t
  threads hazard pointers are are pointing to it before it was not 
  garunteed to be valid.
- prevent freeing nodes that are pointed to by one or more hazard pointers

### Method
- keeps a list of R retired nodes
- if the list is full it checks hazard pointers of threads for matches of the 
  retired nodes and removes / frees any node that has no pointers pointing to it
    - keeps a snapshot of non null hazard pointers in a hash table to make constant time




    

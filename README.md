# IPC Using Signals - Process Manager (mgr)

## Overview
This project implements a **process manager (mgr.c)** in C++ that handles inter-process communication (IPC) using **signals**. The manager (`mgr`) controls multiple child processes (jobs), allowing users to:
- Start new jobs (`r`)
- Suspend jobs (`^Z` - SIGTSTP)
- Resume suspended jobs (`c` - SIGCONT)
- Kill suspended jobs (`k` - SIGKILL)
- Print the process table (`p`)
- Quit while ensuring cleanup (`q`)

Each job prints a character repeatedly for 10 seconds before exiting.

## Features
- **Process Table Management**: Tracks up to 10 child jobs and their statuses.
- **Custom Signal Handling**: Prevents manager (`mgr`) from terminating due to `^C` (SIGINT) or `^Z`.
- **Foreground Execution**: Jobs run in the foreground and receive proper signal forwarding.
- **Process Group Management**: Uses `setpgid()` to separate job groups.
- **Graceful Exit**: Ensures all suspended jobs are killed before quitting.

## Compilation & Execution
### 1. Compile the Programs
```sh
gcc -Wall -o job job.c
g++ -Wall -o mgr mgr.c
```

### 2. Run the Manager
```sh
./mgr
```

## Commands
| Command | Description |
|---------|-------------|
| `p` | Print the process table |
| `r` | Start a new job |
| `c` | Resume a suspended job |
| `k` | Kill a suspended job |
| `h` | Display help message |
| `q` | Quit the manager |

## Signal Handling
| Signal  | Behavior |
|---------|---------------------------------|
| `SIGINT` (`^C`) | Terminates the currently running job |
| `SIGTSTP` (`^Z`) | Suspends the currently running job |
| `SIGCONT` | Resumes a suspended job |
| `SIGKILL` | Kills a suspended job |

## Example Run
```sh
$ ./mgr
mgr> r
Running job M
M M M M M M M M M M
mgr> p
NO    PID         PGID        STATUS          NAME
0     10906       10906       SELF            mgr
1     10909       10909       FINISHED        job M
mgr> r
Running job Y
Y Y ^Z
mgr> p
NO    PID         PGID        STATUS          NAME
0     10906       10906       SELF            mgr
1     10909       10909       FINISHED        job M
2     10910       10910       SUSPENDED       job Y
mgr> q
```

## Future Improvements
- Improve non-blocking handling of `waitpid()` to avoid indefinite waits.
- Enhance user interaction with better error messages.
- Implement a mechanism to reuse process table entries.

## Author
Developed by **[Your Name]**.

## License
This project is licensed under the MIT License. Feel free to modify and distribute.


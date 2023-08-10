# Towards Correctness Checking of MPI Partitioned Communication in MUST - Artifact

This repository contains the artifact with the error test cases for MPI partitioned communications.
The different test cases are in the [tests/](tests/) folder.

## Building Docker Container
The docker container installs MPICH and MUST and copies the test cases in the container.
To build the container, run in the root directory

```
docker build -t must-partitioned .
```

## Running Tests
To run the tests with MUST, execute:

```
docker run -it must-partitioned lit -j1 tests/
```

In case you want to have verbose output of the test runs, run:

```
docker run -it must-partitioned lit -j1 -a tests/
```

## Manually testing single test cases
Start a shell in the docker container:

```
docker run -it must-partitioned bash
```

Go into `tests` directory and compile any test:

```
cd tests/
mpicxx -g test.cpp -o test
```

Run the test with MUST:

```
# Log to stdout
mustrun -np 2 --must:partitioned_verbose_report --must:output stdout ./test
# Log to HTML file
mustrun -np 2 --must:partitioned_verbose_report --must:output html ./test
```
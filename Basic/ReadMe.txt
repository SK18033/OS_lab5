/*=============================================================================

CS 50300 Operating Systems Lab5a - Multi-Threaded program (Basic Elevator)

elevator.c Source code		

Kartik Sooji (ksooji)

=============================================================================*/

There are following functionalities that are supported by the program:

void* person_generator()
void* elevator()
main(int argc, char* argv[])


get_rand(int n):
This function is used to randomly generate the numbers.

person_generator():
~~~~~~~~~~~
This function dynamically creates the number of persons who request to travel in the elevator with the in the range of the floors that the users provide
There are few features related to a person about the floor to floor movement details that are randomly generated
A person is created based on the arrival time mentioned
A doubly linked list is maintained to save the persons generated

elevator(double ***Mat):
~~~~~~~~~~~~~~~~~~~~~~~~
This function creates multiple elevators for the persons created to serve the same as requested.
Each elevator has an elevator speed defined by the user
The elevator picks up a person from the requested floor and drops the person at requested floor

main(int argc, char* argv[]):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-This function reads and validates the user inputs.
-Creates the threads to operate Elevators and also persons.
-Calls both the Person_generator() and elevator() functions.
-The persons are moved accordingly and expected output is printed.


Exceution Steps :

1) Change the current working directory to the source code location
2) Make sure there is a ".c" files
3) Type make and hit enter
4) Provide appropriate inputs as prompted
5) Once the program is done run make clean 

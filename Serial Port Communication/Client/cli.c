#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define clear() printf("\033[H\033[J")
#include<readline/readline.h>


// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()


//Read and print help info from help.txt
void Process_input_help(char *input_str){

    FILE *fptr;
    char c;

    printf("Availabe AT Commands:\n");
    fptr = fopen("help.txt", "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
  
    // Read contents from file
    c = fgetc(fptr);
    while (c != EOF)
    {
        printf ("%c", c);
        c = fgetc(fptr);
    }
  
    fclose(fptr);

}




void Write_Input(int serial_port,char *input,char *text){

    int loop_count,i,step=8;

    //REAd the first 8 bytes
    write(serial_port, input, sizeof(input));   

    //REAd the remaining bytes of the input
    if (strlen(input)>8){
        loop_count=strlen(input)/8;
        for(i=0;i<loop_count;i++){      
            memcpy(text,&input[step],8);
            write(serial_port,text,sizeof(text));
            step=step+8;
        }
    }

}

//print sth for the user
void init_shell(){
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****CLI****");
    printf("\n\n\n****press 'exit' to leave shell****");
    printf("\n\n\n\n*******************"
        "***********************");
    printf("\n");
    sleep(2);
    clear();
}


int main(){


    //initilazation and allocating memory
    //256 bytes max length of the buffer of the device channel
    
    char *input,*text,*input_add,*input_list_values;
    input = (char*)malloc(sizeof(char)*256);
    text = (char*)malloc(sizeof(char)*256);  
    input_add = (char*)malloc(sizeof(char)*256);  
    input_list_values = (char*)malloc(sizeof(char)*256);  


    char return_str1[256];

    //open device
    int serial_port=open("/dev/pts/4",O_RDWR );

    //open cli interface
    init_shell();
    while (1) {
        input = readline("\n>>> ");

        if(strcmp(input,"exit")==0){
            write(serial_port, input, sizeof(input)); //write exit to server
            break;
        }
        if(strcmp(input,"help")==0){   //Process help command
            Process_input_help(input);
            continue;
        }

        if(strcmp(input,"AT+ADD")==0){

            write(serial_port, input, sizeof(input)); 

            read(serial_port, &return_str1, sizeof(return_str1)); 
            printf("%s\n",return_str1);    

            input_add= readline("\n>>> ");                       //Give the integer for the register

            Write_Input(serial_port,input_add,text);   //write to server the int

            read(serial_port, &return_str1, sizeof(return_str1));
            printf("%s\n",return_str1);  

            input_list_values= readline("\n>>> ");    //Give the list of allowed values for the register

            Write_Input(serial_port,input_list_values,text); //write to server the list

            read(serial_port, &return_str1, sizeof(return_str1));
            printf("%s\n",return_str1);              //print final response from server

            continue;
            
        }

        Write_Input(serial_port,input,text);    //write the AT+CMD to the server 

        read(serial_port, &return_str1, sizeof(return_str1)); //Read the response and print it
        printf("%s\n",return_str1);
    }

    close(serial_port);

    free(input);
    free(text);
    free(input_add);
    free(input_list_values);
    return 0;

}

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<readline/readline.h>
#include <ctype.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <regex.h>


int check_input_commands(char *str1){

    int value[3];
    int status_r;
    int status[3];

    regex_t regex3;
    regex_t regex2;
    regex_t regex1;


    // check the input with regular expression

    regcomp(&regex1, "^AT\\+REG[0-9]+$", REG_EXTENDED);
    regcomp(&regex2, "^AT\\+REG[0-9]+=\\?$", REG_EXTENDED);
    regcomp(&regex3, "^AT\\+REG[0-9]+=-?[0-9]+$", REG_EXTENDED);
    
    
    value[0] = regexec(&regex1,str1,0, NULL, 0);
    value[1] = regexec(&regex2,str1,0, NULL, 0);
    value[2] = regexec(&regex3,str1,0, NULL, 0);
    

    for(int i=0;i<3;i++){
          // If pattern found
        if (value[i] == 0) {
            status[i]=1;
        }
     
        // If pattern not found
        else if (value[i] == REG_NOMATCH) {
            status[i]=0;
        }
    }

    if(status[0]==1 || status[1]==1 || status[2]==1){
        status_r=1;
    }
    else{
        status_r=0;
    }

    //return if we found the cmd or no
    return(status_r);
}

//based of the CMD do the one the 3 options and return the output to the client
void execute_commands(int id_command,int reg_position_int,int* reg_values,char** reg_list_val,char* result,int serial_port){

    int result_num;
    char output[256];

    if(id_command==1){
        sprintf(output,"%d\n",reg_values[reg_position_int-1]);
        write(serial_port,output,sizeof(output));
    }
    if(id_command==2){
        sprintf(output,"%s\n",reg_list_val[reg_position_int-1]);
        write(serial_port,output,sizeof(output));
    }
    //In case you add new reg and you want to check the input
    //add an option here to validate the register
    if(id_command==3){
        result_num=atoi(result);
        if(reg_position_int-1==0){
            if(result_num==1 || result_num==2 || result_num==3){
                reg_values[reg_position_int-1]=result_num;
                memcpy(output,"OK",sizeof(output));
                write(serial_port,output,sizeof(output));
            }
            else{
                memcpy(output,"InvalidInput",sizeof(output));
                write(serial_port,output,sizeof(output));
            }

        }
        else if(reg_position_int-1==1){
            if(result_num>=0 && result_num<=16555){
                reg_values[reg_position_int-1]=result_num;
                memcpy(output,"OK",sizeof(output));
                write(serial_port,output,sizeof(output));
            }
            else{
                memcpy(output,"InvalidInput",sizeof(output));
                write(serial_port,output,sizeof(output));
            }

        }
        else{
            reg_values[reg_position_int-1]=result_num;
            memcpy(output,"OK",sizeof(output));
            write(serial_port,output,sizeof(output));
        }
    }

}

int position_of_reg(char *str1,char *reg_position){
    int i,w=0,reg_position_int;

    for(i=0;i<strlen(str1);i++){ //gather position of register
        if(isdigit(str1[i])){
           reg_position[w]=str1[i];
           w=w+1;
        }
        else if(str1[i]=='='){
            break;
        }     
        
    }

    reg_position_int=atoi(reg_position);
    return(reg_position_int);
}


//find which type of commmand is
int find_command(char *str1,char *result){

    int i,j,id_command,k=0;

    for (i=0;i<strlen(str1);i++){  
        if(str1[i]=='='){
            if(str1[i+1]=='?'){
                id_command=2;
                break;
            }
            else{
                for(j=i;j<strlen(str1);j++){
                    result[k]=str1[j+1];
                    k=k+1;
                }
                id_command=3;
                break;
            }
        }
        else{
            id_command=1;
        }
    }
    return(id_command);

}

int main(int argc, char** argv)
{

    //Initiliazation 
    char *result,*reg_position;

    //if we custom add new reg edit code size+1 
    int i,SIZE=2,SIZE_LIST=256,add_reg_value_int;//the value of the AT+REG<int>=add_reg_value_int

    int reg_position_int,id_command,status; 
    //id command shows which type command is,
    //reg_position is the 4 of AT+REG4 as an integer
    //result_num is AT+REG1=15 is the 15

    int *reg_values; //the values of the registers
    char **reg_list_val; // //the list of allowed values

    char str1 [256];  //input from client
    char output[256];   //output from server
    char add_reg_value[256]; //the value of the new register
    char add_reg_list[256]; //the list of allowed values for new register, only for pop up to user no check from server 


     //Initiliazation of registers
    reg_values = (int*)malloc(SIZE*sizeof(int)); //only 2 registers at start
    reg_list_val = (char**)malloc(SIZE*sizeof(char*)); //list of allowed values 
    result=(char*) malloc(256*sizeof(char)); //value to assign at-reg->value in char
    reg_position=(char*)malloc(256*sizeof(char)); //reg_position_int in char

    for (i=0; i<SIZE; i++){ //for the size of regs allocate the lists
        reg_list_val[i]=(char *)malloc(SIZE_LIST*sizeof(char));
    }
  
    //add here the list and value of new reg in case you edit code for new regs
    reg_list_val[0]="1|2|3";
    reg_list_val[1]="0-16555";
    reg_values[0]=1;
    reg_values[1]=1000;

    //open port for server
    int serial_port=open("/dev/pts/3",O_RDWR );
   
    while(1){    
        //if exit terminate server
        read(serial_port, &str1, sizeof(str1));
        if(strcmp(str1,"exit")==0){
            break;
        }

        //Do the extra add reigster command
        if(strcmp(str1,"AT+ADD")==0){
         
            int valid;
            regex_t regex;
            int value_r;


            sprintf(output,"Give integer for the register\n");
            write(serial_port,output,sizeof(output));

            //read the integer of the new reg
            read(serial_port, &add_reg_value, sizeof(add_reg_value));

            //check if the input is int
            regcomp(&regex, "^-?[0-9]+$", REG_EXTENDED);
            value_r = regexec(&regex,add_reg_value,0, NULL, 0);
            // If pattern found
            if (value_r == 0) {
                valid=1;
            }  
            // If pattern not found
            else if (value_r == REG_NOMATCH) {
                valid=0;
            }

            //if value is int store it to array of registers and ask the user for allowed values
            if(valid==1){

                SIZE=SIZE+1;
                //allocate new memory for new

                reg_values= (int *) realloc(reg_values, SIZE*sizeof(int));
                reg_list_val = (char**)realloc(reg_list_val,SIZE*sizeof(char*));
                reg_list_val[SIZE-1]=(char *)realloc(reg_list_val[SIZE-1],SIZE_LIST*sizeof(char));

                add_reg_value_int=atoi(add_reg_value);
                reg_values[SIZE-1]=add_reg_value_int;

                sprintf(output,"Give the list of all allowed values\n");
                write(serial_port,output,sizeof(output));

                read(serial_port, &add_reg_list, sizeof(add_reg_list));
                
                strcpy(reg_list_val[SIZE-1],add_reg_list);
                
                sprintf(output,"OK\n");
                write(serial_port,output,sizeof(output));
                
                memset(str1,0,sizeof(char)*256);

                
            }
            //if not int ask user again to AT+ADD and give int 
            else{

                sprintf(output,"Wrong input,pleage give integer.Press Enter and try again\n");
                write(serial_port,output,sizeof(output));

                read(serial_port, &add_reg_list, sizeof(add_reg_list));
                   
                sprintf(output,"Type AT+ADD and try again\n");
                write(serial_port,output,sizeof(output));
                
                memset(str1,0,sizeof(char)*256);

               

            }
             continue;
        }

        //validate the input if the input follow the format of the AT+CMD
        status=check_input_commands(str1);
        
        if(status==0){
            sprintf(output,"Invalid command\n");
            write(serial_port,output,sizeof(output));           
            continue;
        }
        

        memset(reg_position,0,sizeof(char)*256);
        memset(result,0,sizeof(char)*256);

        //find which of the 3 commands is the input
        id_command=find_command(str1,result);

        //find the position of the register e.g. AT+REG4 the 4
        reg_position_int=position_of_reg(str1,reg_position);

        //if the position of int is out of the scale of the current registers reject it
        if(reg_position_int>SIZE){
            sprintf(output,"REG%d is invalid",reg_position_int);
            write(serial_port,output,sizeof(output));
            memset(str1,0,sizeof(char)*256);
            continue;
        }

        //execute the CMD of the client
        execute_commands(id_command,reg_position_int,reg_values,reg_list_val,result,serial_port);


        memset(str1,0,sizeof(char)*256);
    }      


    close(serial_port);

    free(result);
    free(reg_position);
    free(reg_values);
    free(reg_list_val);


    return 0;
}
    
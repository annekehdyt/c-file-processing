#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <unistd.h>
#include    <fcntl.h>

#define eNOERROR    0
#define eERROR      -1
#define eOUTOFRANGE -2

#define	MAXBUFFSIZE	512
#define MAXNAMELEN  80
#define MAXSTRLEN   200
#define PHONESIZE   20

#define	DELIM "|"

#define PRINT_PEOPLE(p, i) \
    {fprintf(stderr, "[%d] \tname[%s], number[%d]\n", i, (p).name, (p).number); \
    fprintf(stderr, "\taddress[%s], phoneNo[%s], sex[%s]\n\n", (p).address, \
        (p).phone, ((p).sex == 'm' ? "male" : "female"));}

typedef struct {
    int     number;
    char    name[MAXNAMELEN];
    char    sex;
	char	phone[PHONESIZE];
    char    address[MAXSTRLEN];
}   T_PEOPLE;

/* Variable
*  count               Record Count
*  Avail Head          Address of the first avail list record, -1 return if the data does not exist
*/

typedef	struct {
	int		count;	
	off_t	avail_head;	
}	T_HEADER;

int     	num_people=0;
T_HEADER	header_record;

int     write_people(int, int, T_PEOPLE*);
int     delete_write_people(int, int, T_PEOPLE*);
int     read_people(int, int, T_PEOPLE*);
char    get_menuvalue(char*);
void	unpack_people(T_PEOPLE*, char *);
void    menu();


main(int argc, char *argv[])
{
    FILE        *text_in;
    T_PEOPLE    people_buf;
    int         people_file;
    int         size, err, i, menu_value;
    int         number;
	off_t		rec_addr;
	char		buffer[MAXBUFFSIZE];
	
	if(argc < 4) {
        menu();
        fprintf(stderr, "USAGE: %s input_text_file menu_value index\n", argv[0]);
        exit(1);
    }
    
    if((int)NULL == (people_file = open(argv[1], O_BINARY|O_RDWR))) {
        fprintf(stderr, "can\'t open %s\n", argv[1]);
        exit(1);
    }
    
	/* Read the header and create the num people */
	if(sizeof(header_record) != read(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t read the header record\n");
		exit(1);
	}
	num_people = header_record.count;
	
	switch(get_menuvalue(argv[2])){
    case 'p':
              
              menu_value = atoi(argv[3]);
        
              if(menu_value < 0)          exit(1);
              if(menu_value > 0){
                  /* Search the record base on the given index value*/
                  err = read_people(people_file, menu_value, &people_buf);
                  if(err == eERROR) {
                         fprintf(stderr, "can\'t get the %d\'th people\n", i+1);
                         exit(1);
                  }
            
                  if(err == eNOERROR)     PRINT_PEOPLE(people_buf, menu_value);
              } else {
                  /* return back the file pointer to the first location */
	              if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
	    	          fprintf(stderr, "seek error ...\n");
			          exit(1);
		          }

		          /* Analize all of the the data from the beginning */
                  for(i=0;i < num_people;i++)   {
                      /* Read the length of record */
		              if(sizeof(size) != read(people_file, &size, sizeof(size))) {
                          fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
                          exit(1);
                      }
				      
				      if(size < 0){
                              if(eERROR == lseek(people_file, size * (-1), SEEK_CUR)) {
                                  fprintf(stderr, "seek error ...\n");
			                      exit(1);
                              }
                              i--;
				              continue;
                      }
                      
		              /* Read the record */
		              if(size != read(people_file, buffer, size)) {
		                  fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
				          exit(1);
                      }
                      
                      /* unpack the record and print to the screen */
			          unpack_people(&people_buf, (char*)buffer);
                      PRINT_PEOPLE(people_buf, i+1);
                  }
              } 
              break;
    case 'n' :
              /* Read the header of the file */
              if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
                  fprintf(stderr, "seek error ...\n");
			      exit(1);
              }

		      /* Analize all of the the data from the beginning */
              for(i=0;i < num_people;i++)   {
                  /* Read the length of record */
		          if(sizeof(size) != read(people_file, &size, sizeof(size))) {
                      fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
                      exit(1);
                  }
				
		          /* Read the record */
		          if(size != read(people_file, buffer, size)) {
				      fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
				      exit(1);
		          }
		          
                  /* unpack the record and print to the screen */
			      unpack_people(&people_buf, (char*)buffer);
			      if(!strcmp(argv[3], people_buf.name)){
                                    PRINT_PEOPLE(people_buf, i+1);    
                  }           
              }
              break;
    case 's' :
         
              number = atoi(argv[3]);
              if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
                  fprintf(stderr, "seek error ...\n");
			      exit(1);
              }

		      /* Analize all of the the data from the beginning */
              for(i=0;i < num_people;i++)   {
                  /* Read the length of the record */
		          if(sizeof(size) != read(people_file, &size, sizeof(size))) {
                      fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
                      exit(1);
                  }
				
		          /* Read the record */
		          if(size != read(people_file, buffer, size)) {
				      fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
				      exit(1);
		          }

			      unpack_people(&people_buf, (char*)buffer);
			      if(number == people_buf.number){
                                    PRINT_PEOPLE(people_buf, i+1);    
                  }           
              }
              break;
    case 'd' :
              number = atoi(argv[3]);
              
              if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
                  fprintf(stderr, "seek error ...\n");
			      exit(1);
              }

		      /* Analize all of the the data from the beginning */
              for(i=0;i < num_people;i++)   {
                  /* Read the length of the record */
		          if(sizeof(size) != read(people_file, &size, sizeof(size))) {
                      fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
                      exit(1);
                  }
		          /* Read the record */
		          if(size != read(people_file, buffer, size)) {
				      fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
				      exit(1);
		          }

			      unpack_people(&people_buf, (char*)buffer);
			      if(number == people_buf.number){
                                    /* Move the cursor to the next record position */      
                                    if(eERROR == lseek(people_file, (sizeof(size)+size)*(-1), SEEK_CUR)) {
                                        fprintf(stderr, "seek error ... \n");
		                                return(eERROR);
	                                }
	                                
	                                /* Return the value of the address record*/
                                    rec_addr = delete_write_people(people_file, size,&people_buf);
                                    PRINT_PEOPLE(people_buf, i+1);
                                    
                                    /* Input the count and address value of deleted record
                                    *  to the header */
                                    header_record.count = --num_people; 
                                    header_record.avail_head = rec_addr;
                                    
                                    /* return the cursor pointer to the start position */
                                    if(eERROR == lseek(people_file, 0, SEEK_SET)) {
                                        fprintf(stderr, "seek error ... \n");
		                                return(eERROR);
	                                }
	                                
	                                /* Write the header to the file */
                                    if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                         fprintf(stderr, "can\'t write the header record\n");
		                                 exit(1);
	                                }
	                                
                                    exit(1);    
                  }           
              }
              
              break;
    case 'i' :
                
                if((FILE*)NULL == (text_in = fopen(argv[1], "r")))  {
                    fprintf(stderr, "can't open %s\n", argv[1]);
                    exit(1);
                }
                
                if(eERROR == lseek(people_file, 0, SEEK_END)) {
	    	          fprintf(stderr, "seek error ...\n");
			          exit(1);
		        }
		          
                while(1)    {
                    if(EOF == fscanf(text_in, "%d %s %c %s %s", &(people_buf.number), 
                                people_buf.name, &(people_buf.sex), people_buf.phone,
                                people_buf.address)) break;
                    
                    
            		/* 이진 데이터 파일에 가변길이 형태로 레코드 저장 */
            		if((rec_addr = write_people(people_file, num_people, &people_buf)) == eERROR) {
                    	fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                       	exit(1);
            		}
            		fprintf(stderr, "%d\'th record\'s address : %d\n", num_people+1, rec_addr);
                    num_people++;
                }
                
    default :
              menu(); 
    }

    fprintf(stderr, "END.\n");
}


int read_people(int in_file, int index, T_PEOPLE *people)
{
	int	i, size;
	char buffer[MAXBUFFSIZE];

    if(index > num_people) {
        fprintf(stderr, 
            "\tERROR: out of range, enter the number between 0 and %d\n", num_people);
        return(eOUTOFRANGE);
    }
    
	/* 파일 포인터를 헤더 레코드를 지나서 첫번째 레코드 위치로 조정 */
	if(eERROR == lseek(in_file, sizeof(header_record), SEEK_SET)) {
		fprintf(stderr, "seek error ...\n");
		return(eERROR);
	}

	for(i=0; i < num_people; i++) {
		/* Read the length of the record */
		if(sizeof(size) != read(in_file, &size, sizeof(size))) {
			fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			return(eERROR);
		}
		
		/* Read the record */
		if(size != read(in_file, buffer, size)) {
			fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
			return(eERROR);
		}

		if(i == index-1) break;
	}

	unpack_people(people, (char*)buffer);

    return(eNOERROR);
}

void unpack_people(T_PEOPLE *people, char *buffer)
{
	char	*result=NULL;
  
	/* Copy the value of number without Delim */
	result = strtok(buffer, DELIM);
	people->number = atoi(result);

    /* Copy the value of name without Delim */
	result = strtok(NULL, DELIM);
	strcpy(people->name, result);

    /* Copy the value of sex without Delim */
	result = strtok(NULL, DELIM);
	people->sex = *result;

    /* Copy the value of phone without Delim */
	result = strtok(NULL, DELIM);
	strcpy(people->phone, result);
    
    /* Copy the value of Address without Delim */
	result = strtok(NULL, DELIM);
	strcpy(people->address, result);
}

int	write_people(int out_file, int index, T_PEOPLE *people)
{
	char	buffer[MAXBUFFSIZE];
	int		size;
	int		rec_address;

	/* PACKING : buffer를 필드값과 구분자의 연속으로 구성 */
	sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
	size = strlen(buffer);

	/* 현재 저장할 레코드의 바이트 주소를 확인 */
	rec_address = lseek(out_file, 0, SEEK_CUR);

	/* 레코드 길이를 저장 */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}

	/* 레코드를 저장 */
	if(size != write(out_file, buffer, size)) {
			return(eERROR);
	}

	/* 저장한 레코드의 바이트 주소를 반환 */
	return(rec_address);
}


int	delete_write_people(int out_file, int size, T_PEOPLE *people)
{
	char	buffer[MAXBUFFSIZE];
	int		rec_address;
	int     buffer_size;
    
    people->number = 0;
    strcpy(people->name," ");
    strcpy(people->phone," ");
    strcpy(people->address," ");
    
	/* PACKING : buffer field value and delimiter manage */
	sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
	buffer_size = strlen(buffer);
	size = size * (-1);
	
	/* Confirm the current byte address */
	rec_address = lseek(out_file, 0, SEEK_CUR);
	
	/* Save the length of the record */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}
	/* Save the Record */
	if(buffer_size != write(out_file, buffer, buffer_size)) {
			return(eERROR);
	}
    printf("%d\n", size);
    
    /* Return the address byte*/
	return(rec_address);
}

char get_menuvalue(char *value){
    if(!strcmp(value, "p"))
        return 'p';        
    if(!strcmp(value, "s"))
        return 's';   
    if(!strcmp(value, "n"))
        return 'n';  
    if(!strcmp(value, "d"))
        return 'd';   
    if(!strcmp(value, "i"))
        return 'i';   
}

void menu()
{
    printf("\n\t**********     TYPE OF INPUT     **********\n");
    printf("[FILENAME] - p index_number : Enter the index number(0 ~ %d)\n");
    printf("\t\tif 0, print all people~ \n", num_people);
    printf("[FILENAME] - s student_number\n");
    printf("[FILENAME] - n name\n\n");  
    printf("[FILENAME] - d(delete) student_number\n"); 
    printf("[FILENAME] - i [TEXTFILE]\n\n");  
}

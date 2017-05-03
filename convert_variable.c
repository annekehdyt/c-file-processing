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

/* 구분자 */
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

typedef	struct {
	int		count;	/* 레코드의 수 */
	off_t	avail_head;	/* avail list의 첫번째 레코드 주소, 없으면 -1 */
}	T_HEADER;

int     	num_people=0;
T_HEADER	header_record;

int     write_people(int, int, T_PEOPLE*);
int     read_people(int, int, T_PEOPLE*);
void	unpack_people(T_PEOPLE*, char *);

main(int argc, char **argv)
{
    FILE        *text_in;
    T_PEOPLE    people_buf;
    int         people_file;
    int         size, err, i, menu_value;
	off_t		rec_addr;
	char		buffer[MAXBUFFSIZE];

    if(argc < 3) {
        fprintf(stderr, "USAGE: %s input_text_file output_record_file\n", argv[0]);
        exit(1);
    }

    if((FILE*)NULL == (text_in = fopen(argv[1], "r")))  {
        fprintf(stderr, "can't open %s\n", argv[1]);
        exit(1);
    }

    if((int)NULL >
       (people_file = open(argv[2], O_BINARY|O_TRUNC|O_RDWR|O_CREAT, 0644))) {
        fprintf(stderr, "can't open %s\n", argv[2]);
        exit(1);
    }

	/* 먼저 헤더 레코드를 저장 */
	header_record.avail_head = -1;
	header_record.count = 0;
	if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t write the header record\n");
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
    fprintf(stderr, "%d people data stored ...\n", num_people);

	/* 헤더 레코드의 count 값을 num_people로 수정 */
	header_record.count = num_people;
	header_record.avail_head = -1;

	/* 파일 포인터를 처음으로 옮김 */
	if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		fprintf(stderr, "seek error ...\n");
		exit(1);
	}

	/* 헤더 레코드를 파일에 저장 */
	if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t write the header record\n");
		exit(1);
	}

    fclose(text_in);
    close(people_file);
   
    fprintf(stderr, "END.\n");
    getchar();
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
		/* 레코드 길이를 읽음 */
		if(sizeof(size) != read(in_file, &size, sizeof(size))) {
			fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			return(eERROR);
		}
		
		/* 레코드를 읽음 */
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

	/* 구분자(DELIM)로 구분된 첫번째 필드값인 number 복사 */
	result = strtok(buffer, DELIM);
	people->number = atoi(result);

	/* 구분자(DELIM)로 구분된 두번째 필드값인 name 복사 */
	result = strtok(NULL, DELIM);
	strcpy(people->name, result);

	/* 구분자(DELIM)로 구분된 세번째 필드값인 sex 복사 */
	result = strtok(NULL, DELIM);
	people->sex = *result;

	/* 구분자(DELIM)로 구분된 네번째 필드값인 phone 복사 */
	result = strtok(NULL, DELIM);
	strcpy(people->phone, result);

	/* 구분자(DELIM)로 구분된 다섯번째 필드값인 address 복사 */
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

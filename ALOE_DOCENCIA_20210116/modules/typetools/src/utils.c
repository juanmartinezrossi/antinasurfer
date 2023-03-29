#include "utils.h"

#include "phal_sw_api.h"
#include "phal_hw_api_man.h"


/*void int2char (int in[],int len, char out [])
{
	int i;
	
	for (i=0; i<len; i++)
	{
		out[i]=in[i];
	}
}


void char2float(char in[], int len, float out [])
{
	int i;
	char tmp[2];

	for (i=0; i<len; i++)
	{	
		sprintf(tmp,"%d",in[i]);
		out[i]=(float)(atoi(tmp));
	}

}


void char2int(char in[], int len, int out [])
{
	int i;
	char tmp[2];

	for (i=0; i<len; i++)
	{	
		sprintf(tmp,"%d",in[i]);
		out[i]=atoi(tmp);
	}

}
*/

int readfile(char nombre[],char array[],int length_max)
{
	char i=0;
	int j=0, n;
	int file_len;
	
	
	/*f=fopen(nombre,"r");

	if(f==NULL)
	{
		printf("Error al abrir fichero\n");
		exit(0);           
	}

	fscanf(f,"%c",&i);
        array[j]=i;
	j++;
	
    	while(!feof(f)&&(j<length_max))
     	{
   		fscanf(f,"%c",&i);
             	array[j]=i;
           	j++;
     	}
	
     	fclose(f);
	*/
	int fd;
	
	fd = hwapi_res_open(nombre, FLOW_READ_ONLY, &file_len);
	if (fd<0) {
		printf("Error al abrir el fichero %s\n", nombre);
		return -1;
	}

	if (array) {
		if (hwapi_res_read(fd, array, file_len>length_max?length_max:file_len) < 0) {
			printf("Error leyendo\n");
			return -1;
		}
	}

	hwapi_res_close(fd);

	return file_len;
	
}


void escribe_archivo(char nombre[], unsigned char data[], int length_max)
{
     	int i;
     	FILE *f;
     /*
     	
	f=fopen(nombre,"a"); //Crear fichero
      
     	for(i=0;i<length_max && data[i]!=NULL;i++)
     	{ 
     		fprintf(f,"%c",data[i]);// Escribe 
     	}
     
    	fclose(f);
*/
	int fd;
	
	fd = hwapi_res_open(nombre, FLOW_WRITE_ONLY, NULL);
	if (fd<0) {
		printf("Error al abrir el fichero %s\n", nombre);
		exit(0);
	}

	if (hwapi_res_write(fd, data, length_max) < 0) {
		printf("Error guardando datos\n");
	}

	hwapi_res_close(fd);

}


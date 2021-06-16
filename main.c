#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson/cJSON.h"

// helper data structures to make my life easier
typedef struct
{
   int int1;
   char* str1;
} obj;

void printObj(obj myObj){
   printf("Printing new obj\n");
   printf("int1: %d \n", myObj.int1);
   printf("str1: %s \n", myObj.str1);
}

typedef struct
{
   char* str1;
   char* str2;
   int int1;
   int int2;
   double float1;
   double float2;
   double float3; // this is LAST in the json file but should be parsed here as a test
   int* arrInt;
   int arrIntLen;
   obj* arrObj;
   int arrObjLen;
} sampleData;

void printSampleData(sampleData myData){
   printf("Printing new sample data\n");
   printf("str1: %s \n", myData.str1);
   printf("str2: %s \n", myData.str2);
   printf("int1: %d \n", myData.int1);
   printf("int2: %d \n", myData.int2);
   printf("float1: %f \n", myData.float1);
   printf("float2: %f \n", myData.float2);
   printf("float3: %f \n", myData.float3);

   printf("arrInt: ");
   int i;
   for(int i = 0; i < myData.arrIntLen; i++){
      printf("%d ", myData.arrInt[i]);
   }
   printf("\n");

   printf("arrObj: ");
   for(int i = 0; i < myData.arrObjLen; i++){
      printObj(myData.arrObj[i]);
   }
   printf("\n");
}

/* 
   A variety of helper objects to read from Json and fill up a given data structure
*/

int getFileStr(char* inFile, char** fileStr){
   /*
      Get a full string of the contents of file at path inFile.
      Need to pass this &str for the second argument due to dynamic memory realloc
   */
   printf("Reading file %s \n", inFile);
   
   FILE *fptr;
   // read file in with basic error checking
   if ((fptr = fopen(inFile, "r")) == NULL){ 
      printf("Error opening file %s\nQuitting.\n", inFile);
      return 1;
   }

   /* Get the number of bytes */
   fseek(fptr, 0L, SEEK_END);
   long numbytes = ftell(fptr);
   
   /* reset the file position indicator to 
   the beginning of the file */
   fseek(fptr, 0L, SEEK_SET);	
   
   /* grab sufficient memory for the 
   buffer to hold the text */
   *fileStr = (char*)calloc(numbytes, sizeof(char));	
   
   /* memory error */
   if(*fileStr == NULL)
      return 2;
   
   /* copy all the text into the buffer */
   fread(*fileStr, sizeof(char), numbytes, fptr);

   fclose(fptr); // close file to free memory
   return 0;
}

int readJson(char* inFile){   
   /*
      Reads the json file specified by inFile.
   */
   char* fileStr = NULL;
   if(getFileStr(inFile, &fileStr) != 0){ // read, return on error
      printf("Error reading file %s\n", inFile);
   } 
   printf("==== Read JSON =====\n%s", fileStr);
   printf("\n\n");

   //now can actually parse the json
   cJSON *jObj = cJSON_Parse(fileStr); // create the json object 
   sampleData myData; // create the data object we hope to fill
   if (jObj == NULL) // error checking
   {
      const char *error_ptr = cJSON_GetErrorPtr();
      if (error_ptr != NULL)
      {
         fprintf(stderr, "Json read error. \nError before: %s\n", error_ptr);
      }
      return 1;
   }

   // get strings 
   /// TODO: these have to be heap allocated
   myData.str1 = cJSON_GetObjectItemCaseSensitive(jObj, "string")->valuestring;
   myData.str2 = cJSON_GetObjectItemCaseSensitive(jObj, "string2")->valuestring;

   // get primitives
   myData.int1 = cJSON_GetObjectItemCaseSensitive(jObj, "int")->valueint;
   myData.int2 = cJSON_GetObjectItemCaseSensitive(jObj, "int2")->valueint;
   myData.float1 = cJSON_GetObjectItemCaseSensitive(jObj, "float")->valuedouble;
   myData.float2 = cJSON_GetObjectItemCaseSensitive(jObj, "float2")->valuedouble;
   myData.float3 = cJSON_GetObjectItemCaseSensitive(jObj, "float3")->valuedouble;

   // get int array (can't do custom methods for this, have to parse manually)
   cJSON *arrInt = cJSON_GetObjectItemCaseSensitive(jObj, "arrayInt"); // get array ITEM
   myData.arrIntLen = cJSON_GetArraySize(arrInt); // get array size for parsing
   int i;
   int intBuff[myData.arrIntLen]; // create a temp buffer to hold the int array data items
   for (i = 0; i < myData.arrIntLen; i++) 
      intBuff[i] = cJSON_GetArrayItem(arrInt, i)->valueint; //get the individual int array items
   myData.arrInt = intBuff; // write to data
   cJSON_Delete(arrInt); // need to free this object

   // get obj array (again, have to parse manually but handle the objects differently)
   cJSON *arrObj = cJSON_GetObjectItemCaseSensitive(jObj, "arrayObj");
   myData.arrObjLen = cJSON_GetArraySize(arrObj); // get array size for parsing
   obj objBuff[myData.arrObjLen]; // create a temp buffer to hold the int array data items
   for (i = 0; i < myData.arrObjLen; i++) {
      cJSON *objElem = cJSON_GetArrayItem(arrObj, i); // get the individual cjson object of this elem
      objBuff[i].int1 = cJSON_GetObjectItemCaseSensitive(objElem, "int")->valueint;
      objBuff[i].str1 = cJSON_GetObjectItemCaseSensitive(objElem, "str")->valuestring;
      cJSON_Delete(objElem); // need to free this object
   }
   myData.arrObj = objBuff; // write to data
   cJSON_Delete(arrObj); // need to free this object

   printSampleData(myData); // print the sample data at the end for verification
   cJSON_Delete(jObj); // free the json object
   free(fileStr); // free the file string when done
}

int main() {
   readJson("testCase.json");

   return 0;
}
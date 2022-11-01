/// -------------------------------- My Code -------------------------------- ///

#include <windows.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tlhelp32.h>
#include <unistd.h>

FILE *fpo;
const int output_style = 2;

// Find process with the given application name
int findMyProc(const char *procname) {

  HANDLE hSnapshot;
  PROCESSENTRY32 pe;
  BOOL hResult;
  int count = 0;

  // snapshot of all processes in the system
  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (INVALID_HANDLE_VALUE == hSnapshot)
    return 0;

  // initializing size: needed for using Process32First
  pe.dwSize = sizeof(PROCESSENTRY32);

  // info about first process encountered in a system snapshot
  hResult = Process32First(hSnapshot, &pe);

  // retrieve information about the processes
  // and exit if unsuccessful
  while (hResult) {
    // if we find the process: return process ID
    if (strcmp(procname, pe.szExeFile) == 0) {
      count++;
    }
    hResult = Process32Next(hSnapshot, &pe);
  }

  // closes an open handle (CreateToolhelp32Snapshot)
  CloseHandle(hSnapshot);
  return count;
}

// Try open directory in txt file
char* read_input(char* input_file) {
    char* dir = malloc(511);
    FILE *fp = fopen(input_file, "r");
    if (fp) {
        printf("fp not NULL");
        return fgets(dir,511,fp);
    }
    else return NULL;
}

// Diverse at max depth of 10, concat
int travel(char* file_or_folder, char* root_folder, int depth) {
    if (depth > 10) return 0;

    char* need_to_open = malloc(1024);
    if(depth > 0) sprintf(need_to_open, "%s\\%s", root_folder, file_or_folder);
    else need_to_open = file_or_folder;

    struct stat statbuf;

    DIR *dir = opendir(need_to_open);
    if (!dir) {
        //printf("ERROR: This directory is not valid, or you don't have permission to access!\n%s\n", need_to_open);
        //fprintf(fpo, " (ERROR when trying to open this)");
        if (depth==0) printf("NOOO");
        return (depth > 0? 0: 3);
    }

    char *file_name;
    int file_type;
    struct dirent *dp;
    while ((dp=readdir(dir)) != NULL) {
        // check error path
        if (stat(need_to_open, &statbuf) == -1) {
            fprintf(stderr, "stat error: %s", need_to_open);
            return (depth > 0? 0: 3);
        }

        file_name = dp->d_name;
        ///file_type = dp->d_type;
        //printf("%d", file_type);
        if (strcmp("..", file_name) != 0 && strcmp(".", file_name) != 0) {
            fprintf(fpo,"\n");
            if (output_style == 1) {
                for (int i = 0; i < depth; i++) fprintf(fpo, "  ");
                fprintf(fpo, "- %s", file_name);
            } else if (output_style == 2) {
                fprintf(fpo, "%s\\%s", need_to_open, file_name);
            } else {
                printf("%s\\%s", need_to_open, file_name);
            }
            /// if (file_type == 4) {
            if (S_ISREG(statbuf.st_mode)) {
                return 0;
            }
            if (S_ISDIR(statbuf.st_mode)) {
                if (travel(file_name, need_to_open, 1 + depth) > 0) return 3;
            }
        }
    }
    closedir(dir);
    return 0;
}

int main()
{
    /// ------------ Prevent Double Running ------------- ///
    //getchar();
    if (findMyProc("myapp.exe") > 1) {
        printf("Error: process is already running!!!\n");
        exit(-1);
    }

    /// ---------- Get directory in input file ---------- ///
    if (read_input("inp.txt") == NULL) {
        printf("ERROR: Input file is empty or not existed!!!~~~\n");
        return 1;
    }
    char* input_folder = read_input("inp.txt");
    for (int i = 0; input_folder[i] != '\0'; i++)
        if (input_folder[i] == '\n') {
            input_folder[i] = '\0';
            break;
        }
    printf("%s\n", input_folder);

    /// ---------------- File traversal ---------------- ///
    fpo = fopen("out.txt", "w");
    fprintf(fpo, "Traversal in this directory: %s", input_folder);
    int ret = travel(input_folder, NULL, 0);
    printf("\nDone\n");
    getchar();
    return ret;
    // printf("%d %d %d %d %d %d %d %d", DT_UNKNOWN, DT_REG, DT_DIR, DT_FIFO, DT_SOCK, DT_CHR, DT_BLK, DT_LNK);
}

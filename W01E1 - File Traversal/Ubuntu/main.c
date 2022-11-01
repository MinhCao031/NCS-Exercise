/// -------------------------------- My Code -------------------------------- ///

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

FILE *fpo;
const int output_style = 2;

char* read_input(char* input_file) {
    char* dir = malloc(511);
    FILE *fp = fopen(input_file, "r");
    if (fp) return fgets(dir,511,fp);
    else return NULL;
}

char* concat(char* left, char* right) {
    if (!left) return right;
    //printf("\nCalling function...\n");
    int i,j,k=0;
    char *ans = malloc(255);

    for (i = 0; left[i] != '\0'; i++, k++) {
        ans[k] = left[i];
    }
    for (j = 0; right[j] != '\0'; j++, k++) {
        ans[k] = right[j];
    }
    ans[k] = '\0';
    return ans;
}


int travel(char* file_or_folder, char* root_folder, int depth) {
    if (depth > 10) return 0;

    char* need_to_open = file_or_folder;
    if (depth > 0) need_to_open = concat(root_folder, concat("/", file_or_folder));
    DIR *dir = opendir(need_to_open);
    if (!dir) {
        printf("ERROR: This directory is not valid, or you don't have permission to access!\n%s\n", need_to_open);
        fprintf(fpo, " (ERROR when trying to open this)");
        return (depth > 0? 0: 3);
    }

    char *file_name;
    int file_type;
    struct dirent *dp;
    while ((dp=readdir(dir)) != NULL) {
        file_name = dp->d_name;
        file_type = dp->d_type;
        //printf("%d", file_type);
        if (strcmp("..", file_name) != 0 && strcmp(".", file_name) != 0) {
            fprintf(fpo,"\n");
            if (output_style == 1) {
                for (int i = 0; i < depth; i++) fprintf(fpo, "  ");
                fprintf(fpo, "- %s", file_name);
            } else if (output_style == 2) {
                fprintf(fpo, "%s", concat(need_to_open, concat("/", file_name)));
            } else {
                printf("%s", concat(need_to_open, concat("/", file_name)));
            }
            if (file_type == 4) {
                if (travel(file_name, need_to_open, 1 + depth) > 0) return 3;
            }
        }
    }
    closedir(dir);
    return 0;
}

int main()
{
    /// ---------- Get directory in input file ---------- ///
    if (read_input("inp.txt") == NULL) {
        printf("ERROR: Input file is empty or not existed!\n");
        return 1;
    }
    char* input_folder = read_input("inp.txt");
    for (int i = 0; input_folder[i] != '\0'; i++)
        if (input_folder[i] == '\n') {
            input_folder[i] = 0;
            break;
        }
    printf("%s\n", input_folder);

    /// ---------------- File traversal ---------------- ///
    fpo = fopen("out.txt", "w");
    fprintf(fpo, "Traversal in this directory: %s", input_folder);
    return travel(input_folder, NULL, 0);
    printf("%d %d %d %d %d %d %d %d",
       DT_UNKNOWN, DT_REG, DT_DIR, DT_FIFO,
       DT_SOCK, DT_CHR, DT_BLK, DT_LNK);
}

/// -------------------------------- Solution -------------------------------- ///
/*
#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>

int depth = 10;
int count = 0;

void EnumerateFolders (char* parent)
{
    WIN32_FIND_DATA fd;
    char folder[MAX_PATH];
    sprintf(folder, "%s\\*.*", parent);

    HANDLE hFind = FindFirstFile (folder, &fd);

 	count++;
    printf("count: %d\n", count);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
                {
                    //fprintf (fp, "%s\\%s\n", parent, fd.cFileName);
                    printf ("%s\\%s\n", parent, fd.cFileName);
                    char child[MAX_PATH];
                    sprintf(child, "%s\\%s", parent, fd.cFileName);

                    if (count <= depth) {
					EnumerateFolders (child);
					}
					else{
						count = 0;
					}
                }
            }
            else
            {
                fprintf(fp, "%s\n", fd.cFileName);
                //printf("%s\n", fd.cFileName);
            }
        } while (FindNextFile (hFind, &fd));
        FindClose (hFind);
    }
    //fclose(fp);
}


main()
{

	//freopen("output.txt","w",stdout);
	FILE * fpi = NULL;
    char arr[128];

    //open file input
    fpi= fopen("inp.txt", "r");

    //read
    fgets(arr, 128, fpi);

    //FILE * fp = NULL;
    fp = fopen("output.txt", "w");

    FILE *fp;

    fp = freopen("out.txt", "w+", stdout);

    EnumerateFolders(arr);
    //fclose(fp);
    fclose(fpi);
    fclose(fp);
    getch();
}
*/

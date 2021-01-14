#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>

struct card
{
    char* content;
    int size;
};

struct card **card;
int count = 0;
pthread_mutex_t lock;

void* traverseDir(void* arg) 
{
	char *base_path = arg;
	char path[100];

	// Open directory
	DIR *dir;
	struct dirent *dp;
	struct stat states;
    if ((dir = opendir(base_path)) == NULL) {
        fprintf(stderr, "Cannot open \"%s\": No such file or directory\n", base_path);
        return 0;
    }

	// While loop to traverse directories
	while ((dp = readdir(dir)) != NULL) {
		if (!strcmp(".", dp->d_name) || !strcmp("..", dp->d_name)) {
            continue;
        } 
		else {
			// If file is a directory, then traverse again
			if (dp->d_type == DT_DIR) {
                // Construct new path from our base path
				strcpy(path, base_path);
				strcat(path, "/");
				strcat(path, dp->d_name);
				traverseDir(path);
            } 
			else if (strstr(dp->d_name, "mp3") != NULL){
				if (pthread_mutex_lock(&lock)) {
					fprintf(stderr, "[!] LOCKING ERROR\n");
					exit(-1);
				}
				
				// Get path of file
				strcpy(path, base_path);
				strcat(path, "/");
				strcat(path, dp->d_name);

				lstat(path, &states);

				// Get index
				char *end = dp->d_name + strlen(dp->d_name);
				while (end > dp->d_name) {
					--end;
				}
				if (end > dp->d_name) {
					*end = '\0';
				}
				int index = atoi(dp->d_name);

				// If count is smaller than index, then make count equal to index
				if (count < index) {
					count = index;
				}

				// Allocate memory
				card = realloc(card, (count + 1) * sizeof(struct card *));
				card[index] = malloc(sizeof(struct card));
				card[index]->content = malloc(states.st_size * sizeof(char *));
				
				// Open the mp3 file for reading
				FILE *in = fopen(path, "rb");

				// Read data
				fread(card[index]->content, states.st_size, 1, in);
				card[index]->size = states.st_size;

				// Close the file
    			fclose(in);

				if (pthread_mutex_unlock(&lock)) {
					fprintf(stderr, "[!] UNLOCKING ERROR\n");
					exit(-2);
				}
			}
		}
	}

	// Close directory
	closedir(dir);

	return 0;
}

int main(int argc, char **argv)
{
	card = NULL;
	char *inputFile = argv[1];
	char *outputFile = argv[2];
	int file_count = 0;
	DIR *dir;
	struct dirent *dp;
	char paths[32][100];

	// Open directory
    if ((dir = opendir(inputFile)) == NULL) {
        fprintf(stderr, "Cannot open \"%s\": No such file or directory\n", inputFile);
        return 0;
    }
	// Get file count and paths
	while ((dp = readdir(dir)) != NULL) {
		if (!strcmp(".", dp->d_name) || !strcmp("..", dp->d_name)) {
            continue;
        } 
		else if (dp->d_type == DT_DIR) {
			strcpy(paths[file_count], inputFile);
			strcat(paths[file_count], "/");
			strcat(paths[file_count], dp->d_name);
			file_count++;
		}
	}

	if (pthread_mutex_init(&lock, NULL)) {
        fprintf(stderr, "[!] INIT ERROR\n");
        exit(-3);
    }

	// Spawn threads
	pthread_t thread_id[file_count];
    for (int i = 0; i < file_count; ++i) {
        pthread_create(&thread_id[i], NULL, traverseDir, paths[i]);
    }
	// Wait for threads to finish
    for (int i = 0; i < file_count; ++i) {
        pthread_join(thread_id[i], NULL);
    }

	FILE *out = fopen(outputFile, "wb");

	// For loop to write binary data into outputFile
	for (int k = 0; k < count + 1; k++) {
		fwrite(card[k]->content, sizeof(char), card[k]->size, out);
	}
	
	// Free allocated memory
	for(int k = 0; k < count + 1; k++) {
		free(card[k]->content);
		free(card[k]);
	}
	free(card);
	// Close the output file and dir
    fclose(out);
	closedir(dir);

	pthread_mutex_destroy(&lock);

    return 0;
}

#include <stdio.h>
#include <string.h>

#define MAX_USERS 64
#define NUM_USERS 10
#define NUM_REGIONS 11
#define USERNAME_SZ 64 // according to the rules: usernames can only be between 0 - 15 charancters in length
#define REGION_NAME_SZ 64

typedef struct {
	char current_user[USERNAME_SZ];
	//const u8 user_ids[64]; // alternatively, we can ask for the users id
	char *song_name;
	char song_region[REGION_NAME_SZ]; 
	char owner_id; // who owns this song
	// the song shared list may need to go here
	char *song_shared;
	//char song_shared[NUM_USERS]; // the song can be shared with as many users as the device is provisioned for
} song_structure;

// A Box: Test Structure
/*typedef struct {
	int length; 
	int width;
	int height;
	char contents[30];
} box;*/

int main()
{
	// The regions this device has been provisioned for
	const char *REGION_NAMES[NUM_REGIONS] = { "United States", "Haiti", "Ethiopia", "Jamaica", "Nigeria", "Cameroon", "Costa Rica", "Canada", "United Kingdom", "Germany", "Liberia" };
	// The users for this device
	const char *USERNAMES[NUM_USERS] = { "aaron", "gian", "denzel", "peter", "otily", "paige", "marcial", "tsion", "jay", "edmund" };
	const char lets_share[9][1000] = {"otily", "marcial", "jay", "peter", "gian", "edmund", "denzel", "tsion", "paige"};
	
	/*Define Contents of the Box
	box mybox = {5, 6, 3, "strawberries"};
	int volume;
	// Calculate the Volume of the Box: Volume(Box) = L*W*H
	volume = mybox.length * mybox.width * mybox.height;

	//printf("%d\n", mybox.length);
	printf("The volume of the box is %d\n", volume);
	printf("This box contains %s\n", mybox.contents);*/

	// Let's begin testing the song structure
	song_structure test = {"gian", "Fanfare60", "United States", 0, "gian"}; // what if the song was shared with more people (i.e. , otily, marcial, jay)
	/*char store[9][1000];
	for (int i = 0; i < NUM_USERS; i++) {
		strcpy(store[i], lets_share[i]);
	}*/
	printf("The current user is %s\n", test.current_user);
	printf("The song in the query is %s\n", test.song_name);
	printf("This song has been provsioned for %s\n", test.song_region);
	// if (test.owner_id == 0) printf("Aaron owns the song %s\n", test.song_name); // testing ...
	printf("%s is the owner of the song\n", USERNAMES[test.owner_id]); 
	printf("The song has been shared with the following: %s\n", test.song_shared);
	
	/*for (int i = 0; i < NUM_USERS; ++i)
	{
		printf("%s\n", store[i]);
	}*/

	// printf("%s\n",REGION_NAMES[1]); // View the second entry (i.e. 0, 1 .. ) in the array for the regions; this is Haiti

	// This code allows users to play the full song only if the song region matches one of the regions the 
	// device has been provisioned and the current user is the song owner or when the current user is on the song shared list
	for (int i = 0; i < NUM_REGIONS; i++) {
		if ((strcmp(test.song_region,REGION_NAMES[i]) == 0) && ((strcmp(test.current_user,USERNAMES[test.owner_id]) == 0) || (strcmp(test.current_user,test.song_shared) == 0))) // if the user and the song satisfy both of these conditions
		{
		 	// they may unencrypt the protected song and enjoy the full version
		 	// Get this from Peter's contribution
		 	printf("Here's the full song!! ;)\n"); // temporary
		}
		else
		{
		 	// otherwise, play only the 30 second demo
		 	// Get this from Gian's contribution 
		 	printf("30 Second Demo Only! :(\n"); // temporary
		}
		break;
	}

/*	const char *share[NUM_USERS] = {"gian", "otily", "jay", "marcial"};
	for (int i = 0; i < NUM_USERS; i++) {

		strcpy(test.song_shared[i],share[i]);
	} */
	//printf("%s ", test.song_shared[i]);

	return 0;
}

#include<stdio.h>
/*this is just to create an array that containt the list of users*/

struct SongUsers{
	char *user_name;
	int user_id;
	char *user_region;
	int user_pin;
};
int main()
{
/*this is to get the information of one user only */
	struct SongUsers user;
/*We are assigning values to each user*/

	user.user_name = "otily";
	user.user_id = 1234555;
	user.user_region = "United States";
	user.user_pin = 01234567;

/*this is to display the information of each user*/

printf("\nUser name is: %s", user.user_name);
printf("\nUser Id is: %d", user.user_id);
printf("\nUser region is: %s", user.user_region);
printf("\nUser pin is: %d\n", user.user_pin);

return 0;

}
	




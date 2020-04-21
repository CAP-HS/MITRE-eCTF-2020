
#ifndef SECRETS_H
#define SECRETS_H

#define NUM_REGIONS 3
const char *REGION_NAMES[] = { "United States", "Japan", "Australia" };
const u8 REGION_IDS[] = { 0, 1, 2 };

#define NUM_PROVISIONED_REGIONS 2
const u8 PROVISIONED_RIDS[] = { 0, 1 };

#define NUM_USERS 3
const char *USERNAMES[] = { "drew", "ben", "misha" };
const u8 USER_IDS[] = { 0, 1, 2 };

#define NUM_PROVISIONED_USERS 3
const u8 PROVISIONED_UIDS[] = { 0, 1, 2 };
const char *PROVISIONED_PINS[] = { "840985e075a77daa2cf07c8884da56692391baafdd47644c87a1fd298fde350a", "3ccbf63a58cae1ae64dbbbc293d0bdeede3f278b78ee0e2d8a1ddcd6bdc77c84", "78fa674b2bcc9276bda999c35f5106898079c1e3addd5dc7487de4de872bb5b3" };

#endif // SECRETS_H

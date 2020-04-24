/*
 * eCTF Collegiate 2020 MicroBlaze Example Code
 * Audio Digital Rights Management
 */
#include "setsecs2.h"
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xstatus.h"
#include "xaxidma.h"
#include "xil_mem.h"
#include "util.h"
#include "secrets.h"
#include "xintc.h"
#include "constants.h"
#include "sleep.h"
//#include "xil_sprintf.h"

#include <sys/stat.h>
//////////////////////// GLOBALS ////////////////////////


// audio DMA access
static XAxiDma sAxiDma;

// LED colors and controller
u32 *led = (u32*) XPAR_RGB_PWM_0_PWM_AXI_BASEADDR;
const struct color RED =    {0x01ff, 0x0000, 0x0000};
const struct color YELLOW = {0x01ff, 0x01ff, 0x0000};
const struct color GREEN =  {0x0000, 0x01ff, 0x0000};
const struct color BLUE =   {0x0000, 0x0000, 0x01ff};

// change states
#define change_state(state, color) c->drm_state = state; setLED(led, color);
#define set_stopped() change_state(STOPPED, RED)
#define set_working() change_state(WORKING, YELLOW)
#define set_playing() change_state(PLAYING, GREEN)
#define set_paused()  change_state(PAUSED, BLUE)

// shared command channel -- read/write for both PS and PL
volatile cmd_channel *c = (cmd_channel*)SHARED_DDR_BASE;

// internal state store
internal_state s;

// structure 
size_t load_file(char *fname, char *song_buf) {
    int fd;
    struct stat sb;

    fd = open(fname, 00);
    if (fd == -1){
        mb_printf("Failed to open file! Error \r\n");
        return 0;
    }

    if (fstat(fd, &sb) == -1){
        mb_printf("Failed to stat file! Error \r\n");
        return 0;
    }

    read(fd, song_buf, sb.st_size);
    close(fd);

    mb_printf("Loaded file into shared buffer (%dB)\r\n", sb.st_size);

    memset((void*)c->songname,0,64);
    Xil_MemCpy((void*)c->songname,fname,64);	//Keep track of song name
    return sb.st_size;
}
//////////////////////// INTERRUPT HANDLING ////////////////////////


// shared variable between main thread and interrupt processing thread
volatile static int InterruptProcessed = FALSE;
static XIntc InterruptController;

void myISR(void) {
    InterruptProcessed = TRUE;
}


//////////////////////// UTILITY FUNCTIONS ////////////////////////


// returns whether an rid has been provisioned
int is_provisioned_rid(char rid) {
    for (int i = 0; i < NUM_PROVISIONED_REGIONS; i++) {
        if (rid == PROVISIONED_RIDS[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// looks up the region name corresponding to the rid
int rid_to_region_name(char rid, char **region_name, int provisioned_only) {
    for (int i = 0; i < NUM_REGIONS; i++) {
        if (rid == REGION_IDS[i] &&
            (!provisioned_only || is_provisioned_rid(rid))) {
            *region_name = (char *)REGION_NAMES[i];
            return TRUE;
        }
    }

    mb_printf("Could not find region ID '%d'\r\n", rid);
    *region_name = "<unknown region>";
    return FALSE;
}


// looks up the rid corresponding to the region name
int region_name_to_rid(char *region_name, char *rid, int provisioned_only) {
    for (int i = 0; i < NUM_REGIONS; i++) {
        if (!strcmp(region_name, REGION_NAMES[i]) &&
            (!provisioned_only || is_provisioned_rid(REGION_IDS[i]))) {
            *rid = REGION_IDS[i];
            return TRUE;
        }
    }

    mb_printf("Could not find region name '%s'\r\n", region_name);
    *rid = -1;
    return FALSE;
}


// returns whether a uid has been provisioned
int is_provisioned_uid(char uid) {
    for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
        if (uid == PROVISIONED_UIDS[i]) {
            return TRUE;
        }
    }
    return FALSE;
}


// looks up the username corresponding to the uid
int uid_to_username(char uid, char **username, int provisioned_only) {
    for (int i = 0; i < NUM_USERS; i++) {
        if (uid == USER_IDS[i] &&
            (!provisioned_only || is_provisioned_uid(uid))) {
            *username = (char *)USERNAMES[i];
            return TRUE;
        }
    }

    mb_printf("Could not find uid '%d'\r\n", uid);
    *username = "<unknown user>";
    return FALSE;
}


// looks up the uid corresponding to the username
int username_to_uid(char *username, char *uid, int provisioned_only) {
    for (int i = 0; i < NUM_USERS; i++) {
        if (!strcmp(username, USERNAMES[USER_IDS[i]]) &&
            (!provisioned_only || is_provisioned_uid(USER_IDS[i]))) {
            *uid = USER_IDS[i];
            return TRUE;
        }
    }

    mb_printf("Could not find username '%s'\r\n", username);
    *uid = -1;
    return FALSE;
}


// loads the song metadata in the shared buffer into the local struct
void load_song_md() {
    s.song_md.md_size = c->song.md.md_size;
    s.song_md.owner_id = c->song.md.owner_id;
    s.song_md.num_regions = c->song.md.num_regions;
    s.song_md.num_users = c->song.md.num_users;
    Xil_MemCpy(s.song_md.rids, (void *)get_drm_rids(c->song), s.song_md.num_regions);
    Xil_MemCpy(s.song_md.uids, (void *)get_drm_uids(c->song), s.song_md.num_users);
}


// checks if the song loaded into the shared buffer is locked for the current user
int is_locked() {
    int locked = TRUE;

    // check for authorized user
    if (!s.logged_in) {
        mb_printf("No user logged in");
    } else {
        load_song_md();

        // check if user is authorized to play song
        if (s.uid == s.song_md.owner_id) {
            locked = FALSE;
        } else {
            for (int i = 0; i < NUM_PROVISIONED_USERS && locked; i++) {
                if (s.uid == s.song_md.uids[i]) {
                    locked = FALSE;
                }
            }
        }

        if (locked) {
            mb_printf("User '%s' does not have access to this song", s.username);
            return locked;
        }
        mb_printf("User '%s' has access to this song", s.username);
        locked = TRUE; // reset lock for region check

        // search for region match
        for (int i = 0; i < s.song_md.num_regions; i++) {
            for (int j = 0; j < (u8)NUM_PROVISIONED_REGIONS; j++) {
                if (PROVISIONED_RIDS[j] == s.song_md.rids[i]) {
                    locked = FALSE;
                }
            }
        }

        if (!locked) {
            mb_printf("Region Match. Full Song can be played. Unlocking...");
        } else {
            mb_printf("Invalid region");
        }
    }
    return locked;
}


// copy the local song metadata into buf in the correct format
// returns the size of the metadata in buf (including the metadata size field)
// song metadata should be loaded before call
int gen_song_md(char *buf) {
    buf[0] = ((5 + s.song_md.num_regions + s.song_md.num_users) / 2) * 2; // account for parity
    buf[1] = s.song_md.owner_id;
    buf[2] = s.song_md.num_regions;
    buf[3] = s.song_md.num_users;
    Xil_MemCpy(buf + 4, s.song_md.rids, s.song_md.num_regions);
    Xil_MemCpy(buf + 4 + s.song_md.num_regions, s.song_md.uids, s.song_md.num_users);

    return buf[0];
}



//////////////////////// COMMAND FUNCTIONS ////////////////////////


// attempt to log in to the credentials in the shared buffer
void login() {
    if (s.logged_in) {
        mb_printf("Already logged in. Please log out first.\r\n");
        Xil_MemCpy((void*)c->username, s.username, USERNAME_SZ);
	//Edit input pin
        Xil_MemCpy((void*)c->pin, s.pin, MAX_PIN_SZ);

    } else {
    	char temp[MAX_PIN_SZ];
    	Xil_MemCpy(temp, sha256hash((void*)c->pin,20), MAX_PIN_SZ);

        for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
            // search for matching username
            if (!strncmp((void*)c->username, USERNAMES[PROVISIONED_UIDS[i]],USERNAME_SZ)) {

                // check if pin matches
                if (!strncmp(temp, PROVISIONED_PINS[i],MAX_PIN_SZ)) {
                    //update states
                    s.logged_in = 1;
                    c->login_status = 1;
                    Xil_MemCpy(s.username, (void*)c->username, USERNAME_SZ);
                    Xil_MemCpy(s.pin, (void*)c->pin, MAX_PIN_SZ);
                    s.uid = PROVISIONED_UIDS[i];
                    mb_printf("Logged in for user '%s'\r\n", c->username);
                    return;
                } else {
                    // reject login attempt
                    mb_printf("Incorrect pin for user '%s'\r\n", c->username);
                    memset((void*)c->username, 0, USERNAME_SZ);
                    memset((void*)c->pin, 0, MAX_PIN_SZ);
                    return;
                }
            }
        }

        // reject login attempt
        mb_printf("User not found\r\n");
        memset((void*)c->username, 0, USERNAME_SZ);
        memset((void*)c->pin, 0, MAX_PIN_SZ);
    }
}


// attempt to log out
void logout() {
    if (c->login_status) {
        mb_printf("Logging out...\r\n");
        s.logged_in = 0;
        c->login_status = 0;
        memset((void*)c->username, 0, USERNAME_SZ);
        memset((void*)c->pin, 0, MAX_PIN_SZ);
        s.uid = 0;
    } else {
        mb_printf("Not logged in\r\n");
    }
}


// handles a request to query the player's metadata
void query_player() {
    c->query.num_regions = NUM_PROVISIONED_REGIONS;
    c->query.num_users = NUM_PROVISIONED_USERS;

    for (int i = 0; i < NUM_PROVISIONED_REGIONS; i++) {
        strcpy((char *)q_region_lookup(c->query, i), REGION_NAMES[PROVISIONED_RIDS[i]]);
    }

    for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
        strcpy((char *)q_user_lookup(c->query, i), USERNAMES[i]);
    }

    mb_printf("Queried player (%d regions, %d users)\r\n", c->query.num_regions, c->query.num_users);
}


// handles a request to query song metadata
void query_song() {
    char *name;

    // load song
    load_song_md();
    memset((void *)&c->query, 0, sizeof(query));

    c->query.num_regions = s.song_md.num_regions;
    c->query.num_users = s.song_md.num_users;

    // copy owner name
    uid_to_username(s.song_md.owner_id, &name, FALSE);
    strcpy((char *)c->query.owner, name);

    // copy region names
    for (int i = 0; i < s.song_md.num_regions; i++) {
        rid_to_region_name(s.song_md.rids[i], &name, FALSE);
        strcpy((char *)q_region_lookup(c->query, i), name);
    }

    // copy authorized uid names
    for (int i = 0; i < s.song_md.num_users; i++) {
        uid_to_username(s.song_md.uids[i], &name, FALSE);
        strcpy((char *)q_user_lookup(c->query, i), name);
    }

    mb_printf("Queried song (%d regions, %d users)\r\n", c->query.num_regions, c->query.num_users);
}


// add a user to the song's list of users
void share_song() {
    int new_md_len, shift;
    char new_md[256], uid;

    // reject non-owner attempts to share
    load_song_md();
    if (!s.logged_in) {
        mb_printf("No user is logged in. Cannot share song\r\n");
        c->song.wav_size = 0;
        return;
    } else if (s.uid != s.song_md.owner_id) {
        mb_printf("User '%s' is not song's owner. Cannot share song\r\n", s.username);
        c->song.wav_size = 0;
        return;
    } else if (!username_to_uid((char *)c->username, &uid, TRUE)) {
        mb_printf("Username not found\r\n");
        c->song.wav_size = 0;
        return;
    }

    // generate new song metadata
    s.song_md.uids[s.song_md.num_users++] = uid;
    new_md_len = gen_song_md(new_md);
    shift = new_md_len - s.song_md.md_size;

    // shift over song and add new metadata
    if (shift) {
        memmove((void *)get_drm_song(c->song) + shift, (void *)get_drm_song(c->song), c->song.wav_size);
    }
    Xil_MemCpy((void *)&c->song.md, new_md, new_md_len);

    // update file size
    c->song.file_size += shift;
    c->song.wav_size  += shift;


    //NEW STUFF STARTS HERE---------------

    //Generate shared master secret
	//Retrieve master pub
	char masteruser = "master";
	char masterpub[100];
	Xil_MemCpy(masterpub, retrievePub(masteruser), 100);
	uint8_t outmasterpub[ECC_PUB_KEY_SIZE]; //Contains formatted master pub
	parsepub(masterpub, outmasterpub);

	//Calculate user priv
	char prv[ECC_PRV_KEY_SIZE];
	char hash[64];
	Xil_MemCpy(hash,sha256hash((void*)c->shareduserpin,1),64);
	Xil_MemCpy(prv, hash, ECC_PRV_KEY_SIZE);

	//Generate shared secret
	static uint8_t sharedsec[ECC_PUB_KEY_SIZE];	
	ecdh_shared_secret(prv, masterpub, sharedsec);	

    //Retrieve enc song key from current user
	char songmapname[64+6];
	Xil_MemCpy(songmapname,(void*)c->songname,64);
	char map[6] = "_map";
	strcat(songmapname,map);
	char songenckey[32];
	Xil_MemCpy(songenckey,retrieveEncKey((char *)c->username, songmapname),32);
	uint8_t encsongkey[16]; //Contains encrypted song key formatted
	parsesongK(songenckey, encsongkey);

    //Decrypt current user song key
	//Calculate current user priv
	char currprv[ECC_PRV_KEY_SIZE];
	char currhash[64];
	Xil_MemCpy(currhash,sha256hash((void*)c->pin,1),64);
	Xil_MemCpy(currprv, currhash, ECC_PRV_KEY_SIZE);

	//Generate current user shared secret
	static uint8_t shared[ECC_PUB_KEY_SIZE];	
	ecdh_shared_secret(currprv, masterpub, shared);	

	static uint8_t songK[16];	//Song key	
	struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_decrypt_key(&sk, shared);
	tc_aes_decrypt(songK, encsongkey, &sk);

    //Encrypt song key with shared user secret
 	static uint8_t encsongK[16];	
	struct tc_aes_key_sched_struct sk2;
	(void)tc_aes128_set_encrypt_key(&sk2, sharedsec);
	tc_aes_encrypt(encsongK, songK, &sk2);

    //Setup new line for song map
	char out[16];
	static char out2[16*2];
	memset(out2,0,sizeof(out2));
	unsigned int j;
	for(j = 0; j < sizeof(encsongK); ++j){
		xil_sprintf(out, "%02x",encsongK[j]);
		strcat(out2, out);
	}
	
	char newmapline[64+33];
	Xil_MemCpy(newmapline, (char *)c->username, 64);
	strcat(newmapline, " ");
	strcat(newmapline, out2);

    //Store in song map (We have encrypted song key and username)
	char songmapfile[64+6];
	Xil_MemCpy(songmapfile,(char *)c->songname,64);
	strcat(songmapfile,map);
    	FILE *fp;
	fp = fopen(songmapfile, "w");
	write(fp,newmapline,sizeof(newmapline));
	close(fp);



    mb_printf("Shared song with '%s'\r\n", c->username);
}


// plays a song and looks for play-time commands
void play_song() {
    u32 counter = 0, rem, cp_num, cp_xfil_cnt, offset, dma_cnt, length, *fifo_fill;
    int fd;
    ssize_t count;
    mb_printf("Reading Audio File...");
    load_song_md();

    // get WAV length
    length = c->song.wav_size;
    mb_printf("Song length = %dB", length);


    //This section will replace the following section --------------------------------------
    if (is_locked()){
	//It is locked so load demo version
	char demoname[64+6] = "demo_";
	strcat(demoname,(void*)c->songname);
	load_file(demoname, (void*)&c->song);
	length = PREVIEW_SZ;

    } else {
	//It is unlocked so continue with full song and begin decrypting
	uint8_t tmp[16];
	int k=0,i = 0;
	uint8_t encode[length];
	int offset = 0;
	uint8_t padsong[length];
	uint8_t *padptr = padsong;

	//Retrieve master pub
	char masteruser = "master";
	char masterpub[100];
	Xil_MemCpy(masterpub,retrievePub(masteruser),100);
	uint8_t outmasterpub[ECC_PUB_KEY_SIZE]; //Contains formatted master pub
	parsepub(masterpub, outmasterpub);

	//Calculate user priv
	uint8_t prv[ECC_PRV_KEY_SIZE];
	char hash[64];
	Xil_MemCpy(hash,sha256hash((void*)c->pin,1),64);
	Xil_MemCpy(prv, hash, ECC_PRV_KEY_SIZE);

	//Retrieve enc song key
	//Create song map name by grabbing song name and append '_map' to the end
	char songmapname[64+6];
	Xil_MemCpy(songmapname,(void*)c->songname,64);
	char map[6] = "_map";
	strcat(songmapname,map);
	char songenckey[32];
	Xil_MemCpy(songenckey,retrieveEncKey((char *)c->username, songmapname),32);
	uint8_t encsongkey[16]; //Contains encrypted song key formatted
	parsesongK(songenckey, encsongkey);

	//Generate shared secret
	static uint8_t shared[ECC_PUB_KEY_SIZE];	
	ecdh_shared_secret(prv, masterpub, shared);

	//Decrypt song key
	static uint8_t songK[16];	//Song key	
	struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_encrypt_key(&sk, shared);
	tc_aes_encrypt(songK, encsongkey, &sk);


	//Decrypt song	
	//At this point we have the song in a local buffer padsong
	Xil_MemCpy(padsong,get_drm_song(c->song),length);
	
	struct tc_aes_key_sched_struct sk2;
	(void)tc_aes128_set_decrypt_key(&sk2, songK);


	for(int j = 0; j < length/16;++j)
	{
		if( j + 1 == length/16){	//If this is the last loop
			for (k = 0; k < 16; k++){
				tmp[k] = padptr;
				padptr = padptr + 1;
			}
			tc_aes_decrypt(encode, tmp, &sk2);
		}else{	//This is not the last loop
			for (k = 0; k < 16; k++){
				tmp[k] = padptr;
				//if( j < 16){
				//	printf("%02x ",tmp[k]);
				//}
				padptr = padptr + 1;
			}
			tc_aes_decrypt(encode+offset, tmp, &sk2);
			offset+=16;
  
		}		
		
	}
	Xil_MemCpy(get_drm_song(c->song),encode,length);

    }

    //-----------------------------------------------------------------------------------
    // truncate song if locked ----------------------------------------------------------
    if (length > PREVIEW_SZ && is_locked()) {
        length = PREVIEW_SZ;
        mb_printf("Song is locked.  Playing only %ds = %dB\r\n",
                   PREVIEW_TIME_SEC, PREVIEW_SZ);
    } else {
        mb_printf("Song is unlocked. Playing full song\r\n");
    }
    //-------------------------------------------------------------------------------------
    rem = length;
    fifo_fill = (u32 *)XPAR_FIFO_COUNT_AXI_GPIO_0_BASEADDR;

    // write entire file to two-block codec fifo
    // writes to one block while the other is being played
    set_playing();
    while(rem > 0) {
        // check for interrupt to stop playback
        while (InterruptProcessed) {
            InterruptProcessed = FALSE;

            switch (c->cmd) {
            case PAUSE:
                mb_printf("Pausing... \r\n");
                set_paused();
                while (!InterruptProcessed) continue; // wait for interrupt
                break;
            case PLAY:
                mb_printf("Resuming... \r\n");
                set_playing();
                break;
            case STOP:
                mb_printf("Stopping playback...");
                return;
            case RESTART:
                mb_printf("Restarting song... \r\n");
                rem = length; // reset song counter
                set_playing();
            default:
                break;
            }
        }

        // calculate write size and offset
        cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;
        offset = (counter++ % 2 == 0) ? 0 : CHUNK_SZ;

        // do first mem cpy here into DMA BRAM
        Xil_MemCpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset),
                   (void *)(get_drm_song(c->song) + length - rem),
                   (u32)(cp_num));

        cp_xfil_cnt = cp_num;

        while (cp_xfil_cnt > 0) {

            // polling while loop to wait for DMA to be ready
            // DMA must run first for this to yield the proper state
            // rem != length checks for first run
            while (XAxiDma_Busy(&sAxiDma, XAXIDMA_DMA_TO_DEVICE)
                   && rem != length && *fifo_fill < (FIFO_CAP - 32));

            // do DMA
            dma_cnt = (FIFO_CAP - *fifo_fill > cp_xfil_cnt)
                      ? FIFO_CAP - *fifo_fill
                      : cp_xfil_cnt;
            fnAudioPlay(sAxiDma, offset, dma_cnt);
            cp_xfil_cnt -= dma_cnt;
        }

        rem -= cp_num;
    }
}


// removes DRM data from song for digital out
void digital_out() {
    // remove metadata size from file and chunk sizes
    c->song.file_size -= c->song.md.md_size;
    c->song.wav_size -= c->song.md.md_size;

    if (is_locked() && PREVIEW_SZ < c->song.wav_size) {
        mb_printf("Only playing 30 seconds");
        c->song.file_size -= c->song.wav_size - PREVIEW_SZ;
        c->song.wav_size = PREVIEW_SZ;
    }

    // move WAV file up in buffer, skipping metadata
    mb_printf(MB_PROMPT "Dumping song (%dB)...", c->song.wav_size);
    memmove((void *)&c->song.md, (void *)get_drm_song(c->song), c->song.wav_size);

    mb_printf("Song dump finished\r\n");
}


//////////////////////// MAIN ////////////////////////


int main() {
    u32 status;

    init_platform();
    microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
    microblaze_enable_interrupts();

    // Initialize the interrupt controller driver so that it is ready to use.
    status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    // Set up the Interrupt System.
    status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    // Congigure the DMA
    status = fnConfigDma(&sAxiDma);
    if(status != XST_SUCCESS) {
        mb_printf("DMA configuration ERROR\r\n");
        return XST_FAILURE;
    }

    // Start the LED
    enableLED(led);
    set_stopped();

    // clear command channel
    memset((void*)c, 0, sizeof(cmd_channel));

    mb_printf("Audio DRM Module has Booted\n\r");

    // Handle commands forever
    while(1) {
        // wait for interrupt to start
        if (InterruptProcessed) {
            InterruptProcessed = FALSE;
            set_working();

            // c->cmd is set by the miPod player
            switch (c->cmd) {
            case LOGIN:
                login();
                break;
            case LOGOUT:
                logout();
                break;
            case QUERY_PLAYER:
                query_player();
                break;
            case QUERY_SONG:
                query_song();
                break;
            case SHARE:
                share_song();
                break;
            case PLAY:
                play_song();
                mb_printf("Done Playing Song\r\n");
                break;
            case DIGITAL_OUT:
                digital_out();
                break;
            default:
                break;
            }

            // reset statuses and sleep to allowe player to recognize WORKING state
            strcpy((char *)c->username, s.username);
            c->login_status = s.logged_in;
            usleep(500);
            set_stopped();
        }
    }

    cleanup_platform();
    return 0;
}

#!/usr/bin/env python3
"""
Description: Protects song by adding metadata or any other security measures
Use: Once per song
"""
import json
import struct
import os
import wave
from argparse import ArgumentParser
import numpy as np
from Crypto.Cipher import AES

class ProtectedSong(object):
    """Example song object for protected song"""

    def __init__(self, path_to_song, metadata):
        """initialize values
        Args:
            path_to_song (string): file name where the song to be provisioned is stored
            metadata (bytearray): bytes containing metadata information
        """
        self.song = path_to_song
        self.full_song, self.original_song = self.read_song(path_to_song) 
        #full_song = vals and original_song = song from read_song function
        self.metadata = metadata

    def read_song(self, path, metadata_frames=0):
        """Reads a wave file
        Args:
            path (string): path to song
            metadata_frames (int): if not 0 disregard this number of frames as metadata
        Returns:
            vals (array): integer array of decoded values
            song (Wave Object): wave object associated with entered song
        """
        song = wave.open(os.path.abspath(path), 'r')
        if metadata_frames:
            song.readframes(metadata_frames)  # skip the metadata frames when assigning vals
        vals = np.frombuffer(song.readframes(song.getnframes()), dtype=np.int16)
        #dtype = np.int16 converts a c type int16_t into a np datatype

        key = ['\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37'] #16 byte key
        cipher = ['\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37','\x16', '\x24', '\x12', 'x37'] #16 byte key
       # cipher = AES.new(key, AES.MODE_ECB)

        aes = AES.new(key, AES.MODE_ECB, cipher)
        vals_enc = aes.encrypt(vals)

        song.close()
        return vals_enc, song

    def save_secured_song_to_wave(self, file_location):
        """Saves secured song to wave file assuming all the same characteristics as original song
        Args:
            file_location (string): location to store the file including name"""
        protected_wav = wave.open(os.path.abspath(file_location), 'wb')
        protected_wav.setnchannels(self.original_song.getnchannels())
        protected_wav.setsampwidth(self.original_song.getsampwidth())
        protected_wav.setframerate(self.original_song.getframerate())
        protected_wav.writeframes(self.metadata)

        for val in self.full_song:
            protected_wav_val = struct.pack('<h', vals_enc)
            protected_wav.writeframesraw(protected_wav_val)


    def save_secured_song_to_wave(self, file_location):
        """Saves secured song to wave file assuming all the same characteristics as original song
        Args:
            file_location (string): location to store the file including name"""
        protected_wav = wave.open(os.path.abspath(file_location), 'wb')
        protected_wav.setnchannels(self.original_song.getnchannels())
        protected_wav.setsampwidth(self.original_song.getsampwidth())
        protected_wav.setframerate(self.original_song.getframerate())
        protected_wav.writeframes(self.metadata)

song = wave.open(os.path.abspath(path), 'r')
        if metadata_frames:
            song.readframes(metadata_frames)  # skip the metadata frames when assigning vals
        vals = np.frombuffer(song.readframes(song.getnframes()), dtype=np.int16)
        song.close()

def main():
    parser = ArgumentParser(description='main interface to protect songs')
    parser.add_argument('--region-list', nargs='+', help='List of regions song can be played in', required=True)
    parser.add_argument('--region-secrets-path', help='File location for the region secrets file',
                        required=True)
    parser.add_argument('--outfile', help='path to save the protected song', required=True)
    parser.add_argument('--infile', help='path to unprotected song', required=True)
    parser.add_argument('--owner', help='owner of song', required=True)
    parser.add_argument('--user-secrets-path', help='File location for the user secrets file', required=True)
    args = parser.parse_args()

    regions = json.load(open(os.path.abspath(args.region_secrets_path)))
    try:
        metadata = create_metadata(args.region_list, args.owner, args.user_secrets_path, regions)
    except ValueError:
        raise ValueError('Ensure all user IDs are integers and all regions are in the provided region_information.json')

def create_metadata(regions, user, user_secret_location, region_info):
    """Returns a byte string formatted as follows:
    METADATA_LENGTH(1B)/ownerID(1B)/REGION_LEN(1B)/USER_LEN(1B)/REGIONID1(1B)/REGIONID2 (1B)/.../opt. parity
    Args:
        regions (list): list of regions to provision song for
        user (string): user name for owner of the song
        user_secret_location (string): path to user secrets file
        region_info (dict): mapping of regions provided by region_information.json
    Returns:
        metadata (bytes): bytes of encoded metadata
    Example:
        >>create_metadata(['USA', 'Canada'], 'user1', 'user_secrets.json', {'USA': 1, 'Canada':2})
        'x06/x00/x01/x00/x01/x02'
    """
    user_secrets = json.load(open(os.path.abspath(user_secret_location)))

    # note: metadata must be an even length since each sample is 2B long
    # and ARM processors require memory accesses to be aligned to the type size
    metadata = struct.pack(
        '=3B{regions_length}s{regions_len_2}s'.format(regions_length=len(regions), regions_len_2=len(regions) % 2),
        int(user_secrets[user]['id']), len(regions), 0,
        bytes([region_info[str(r)] for r in regions]),
        b'\x00' if len(regions) % 2 else b'')
         
    return bytes([len(metadata) + 1]) + metadata

#with open("/home/aplus/MITRE-eCTF-2020/sample-audio/Sound-Bite_One-Small-Step.wav", "rb") as f:
#    if metadata_frames:
#            song.readframes(metadata_frames)
#    byte = f.read()
#    while byte != b"":
#        # Do stuff with byte.
#        byte = f.read(1)


if __name__ == '__main__':
    main()
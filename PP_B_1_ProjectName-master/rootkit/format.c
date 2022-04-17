#include <ntddk.h>

#include "vfs.h"
#include "format.h"

//
// BEWARE - HERE BE DRAGONS 
//

NTSTATUS
VFSFormatFAT16(
    IN PVFS_DEVICE_EXTENSION VFSExtension
    )

/*++

  Routine Description:

  	This routine formats the new disk.

  Arguments:
  
  	DeviceObject - Supplies a pointer to the device object that represents
                       the device whose capacity is to be read.

  Return Value:
  
  	status is returned.

--*/

{

    PBOOT_SECTOR_FAT16 bootSector = (PBOOT_SECTOR_FAT16) VFSExtension->DiskImage;
    PUCHAR       firstFatSector;
    ULONG        rootDirEntries;
    ULONG        sectorsPerCluster;
    USHORT       fatType;        // Type FAT 12 or 16
    USHORT       fatEntries;     // Number of cluster entries in FAT
    USHORT       fatSectorCnt;   // Number of sectors for FAT
    PDIR_ENTRY   rootDir;        // Pointer to first entry in root dir

    PAGED_CODE();
    ASSERT(sizeof(BOOT_SECTOR_FAT16) == 512);
    ASSERT(VFSExtension->DiskImage != NULL);

    RtlZeroMemory(VFSExtension->DiskImage, VFSExtension->DiskSize);

    VFSExtension->DiskGeometry.BytesPerSector = 512;
    VFSExtension->DiskGeometry.SectorsPerTrack = 32;     // Using Ramdisk value
    VFSExtension->DiskGeometry.TracksPerCylinder = 2;    // Using Ramdisk value

    //
    // Calculate number of cylinders.
    //

    VFSExtension->DiskGeometry.Cylinders.QuadPart = VFSExtension->DiskSize / 512 / 32 / 2;

    //
    // Our media type is RAMDISK_MEDIA_TYPE
    //

    VFSExtension->DiskGeometry.MediaType = 0xF8;

    KdPrint((
        "Cylinders: %I64d\n TracksPerCylinder: %lu\n SectorsPerTrack: %lu\n BytesPerSector: %lu\n",
        VFSExtension->DiskGeometry.Cylinders.QuadPart, VFSExtension->DiskGeometry.TracksPerCylinder,
        VFSExtension->DiskGeometry.SectorsPerTrack, VFSExtension->DiskGeometry.BytesPerSector
        ));

    rootDirEntries = 224; //VFSExtension->RootDirEntries;
    sectorsPerCluster = 16; //VFSExtension->SectorsPerCluster;

    //
    // Round Root Directory entries up if necessary
    //

    if (rootDirEntries & (DIR_ENTRIES_PER_SECTOR - 1)) {

        rootDirEntries =
            (rootDirEntries + (DIR_ENTRIES_PER_SECTOR - 1)) &
                ~ (DIR_ENTRIES_PER_SECTOR - 1);
    }

    KdPrint((
        "Root dir entries: %lu\n Sectors/cluster: %lu\n",
        rootDirEntries, sectorsPerCluster
        ));

    //
    // We need to have the 0xeb and 0x90 since this is one of the
    // checks the file system recognizer uses
    //

    bootSector->bsJump[0] = 0xeb;
    bootSector->bsJump[1] = 0x3c;
    bootSector->bsJump[2] = 0x90;

    //
    // Set OemName to "RajuRam "
    // NOTE: Fill all 8 characters, eg. sizeof(bootSector->bsOemName);
    //
    bootSector->bsOemName[0] = 'R';
    bootSector->bsOemName[1] = 'a';
    bootSector->bsOemName[2] = 'j';
    bootSector->bsOemName[3] = 'u';
    bootSector->bsOemName[4] = 'R';
    bootSector->bsOemName[5] = 'a';
    bootSector->bsOemName[6] = 'm';
    bootSector->bsOemName[7] = ' ';

    bootSector->bsBytesPerSec = (SHORT)VFSExtension->DiskGeometry.BytesPerSector;
    bootSector->bsResSectors  = 1;
    bootSector->bsFATs        = 1;
    bootSector->bsRootDirEnts = (USHORT)rootDirEntries;

    bootSector->bsSectors     = (USHORT)(VFSExtension->DiskSize /
                                         VFSExtension->DiskGeometry.BytesPerSector);
    bootSector->bsMedia       = (UCHAR)VFSExtension->DiskGeometry.MediaType;
    bootSector->bsSecPerClus  = (UCHAR)sectorsPerCluster;

    //
    // Calculate number of sectors required for FAT
    //

    fatEntries =
        (bootSector->bsSectors - bootSector->bsResSectors -
            bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
                bootSector->bsSecPerClus + 2;

    //
    //
    //

    DbgPrint("[IG Driver:Ramdisk] FAT 16 SELECTED");
    fatType =  16;
    fatSectorCnt = (fatEntries * 2 + 511) / 512;
    fatEntries   = fatEntries + fatSectorCnt;
    fatSectorCnt = (fatEntries * 2 + 511) / 512;

    bootSector->bsFATsecs       = fatSectorCnt;
    bootSector->bsSecPerTrack   = (USHORT)VFSExtension->DiskGeometry.SectorsPerTrack;
    bootSector->bsHeads         = (USHORT)VFSExtension->DiskGeometry.TracksPerCylinder;
    bootSector->bsBootSignature = 0x0;
    bootSector->bsVolumeID      = 0xC0FFFFEE;

    //
    // Set Label to "RamDisk    "
    // NOTE: Fill all 11 characters, eg. sizeof(bootSector->bsLabel);
    //
    bootSector->bsLabel[0]  = 'N';
    bootSector->bsLabel[1]  = 'O';
    bootSector->bsLabel[2]  = ' ';
    bootSector->bsLabel[3]  = 'N';
    bootSector->bsLabel[4]  = 'A';
    bootSector->bsLabel[5]  = 'M';
    bootSector->bsLabel[6]  = 'E';
    bootSector->bsLabel[7]  = ' ';
    bootSector->bsLabel[8]  = ' ';
    bootSector->bsLabel[9]  = ' ';
    bootSector->bsLabel[10] = ' ';

    //
    // Set FileSystemType to "FAT1?   "
    // NOTE: Fill all 8 characters, eg. sizeof(bootSector->bsFileSystemType);
    //
    bootSector->bsFileSystemType[0] = 'F';
    bootSector->bsFileSystemType[1] = 'A';
    bootSector->bsFileSystemType[2] = 'T';
    bootSector->bsFileSystemType[3] = '1';
    bootSector->bsFileSystemType[4] = '?';
    bootSector->bsFileSystemType[5] = ' ';
    bootSector->bsFileSystemType[6] = ' ';
    bootSector->bsFileSystemType[7] = ' ';

    bootSector->bsFileSystemType[4] = ( fatType == 16 ) ? '6' : '2';

    bootSector->bsSig2[0] = 0x55;
    bootSector->bsSig2[1] = 0xAA;

    //
    // The FAT is located immediately following the boot sector.
    //

    firstFatSector    = (PUCHAR)(bootSector + 1);
    firstFatSector[0] = (UCHAR)VFSExtension->DiskGeometry.MediaType;
    firstFatSector[1] = 0xFF;
    firstFatSector[2] = 0xFF;

    if (fatType == 16) {
        firstFatSector[3] = 0xFF;
    }

    //
    // The Root Directory follows the FAT
    //

    rootDir = (PDIR_ENTRY)(bootSector + 1 + fatSectorCnt);

    //
    // Set device name to "MS-RAMDR"
    // NOTE: Fill all 8 characters, eg. sizeof(rootDir->deName);
    //
    
    rootDir->deName[0] = ' ';
    rootDir->deName[1] = ' ';
    rootDir->deName[2] = ' ';
    rootDir->deName[3] = ' ';
    rootDir->deName[4] = ' ';
    rootDir->deName[5] = ' ';
    rootDir->deName[6] = ' ';
    rootDir->deName[7] = ' ';

    rootDir->deExtension[0] = ' ';
    rootDir->deExtension[1] = ' ';
    rootDir->deExtension[2] = ' ';

    rootDir->deAttributes = DIR_ATTR_VOLUME;

    return STATUS_SUCCESS;
}

NTSTATUS
VFSFormatFAT32(
    IN PVFS_DEVICE_EXTENSION VFSExtension
    )

/*++

  Routine Description:
  
  	This routine formats a given buffer or memory mapped file as a FAT32 partition

  Arguments:
  
  	VFSExtension - Supplies a pointer to the device object that represents
	the device whose capacity is to be read.

  Return Value:
  
  	Status of if disk was able to be properly formatted

--*/

{
	/*
    PBOOT_SECTOR_FAT32 bootSector = (PBOOT_SECTOR_FAT32) VFSExtension->DiskImage;

    PAGED_CODE();

    ASSERT(sizeof(BOOT_SECTOR_FAT32) == 512);
    ASSERT(VFSExtension->DiskImage != NULL);

    RtlZeroMemory(VFSExtension->DiskImage, VFSExtension->DiskSize);

    VFSExtension->DiskGeometry.BytesPerSector = 512;
    VFSExtension->DiskGeometry.SectorsPerTrack = 32;  
    VFSExtension->DiskGeometry.TracksPerCylinder = 2;
    
    //
    // Jump instruction to boot code. This is a fake jump, but is needed
    // because it is used by the volume manager to recognize the partition
    // as a FAT volume 
    //

    bootSector->BS_jmpBoot[0] = 0xEB;
    bootSector->BS_jmpBoot[1] = 0x3c;
    bootSector->BS_jmpBoot[2] = 0x90;

    //
    // This is only a name string, Microsoft operating systems don't
    // pay attention to this value, but others might MSWIN4.1 is the
    // recommended value for this. Specification recommends using this
    // value for this field
    //

    bootSector->BS_OEMName[0] = 'M';
    bootSector->BS_OEMName[1] = 'S';
    bootSector->BS_OEMName[2] = 'W';
    bootSector->BS_OEMName[3] = 'I';
    bootSector->BS_OEMName[4] = 'N';
    bootSector->BS_OEMName[5] = '4';
    bootSector->BS_OEMName[6] = '.';
    bootSector->BS_OEMName[7] = '1';

    //
    // Set number of bytes per sector and number of
    // sectors per cluster on the drive.
    //
    
    bootSector->BPB_BytsPerSec = 512;
    bootSector->BPB_SecPerClus = 8;

    //
    // Number of reserved sectors in the Reserved region of the volume
    // starting at the first sector of the volume. This field must not be 0.
    // For FAT12 and FAT16 volumes, this value should never be
    // anything other than 1. For FAT32 volumes, this value is typically
    // 32. There is a lot of FAT code in the world “hard wired” to 1
    // reserved sector for FAT12 and FAT16 volumes and that doesn’t
    // bother to check this field to make sure it is 1. Microsoft operating
    // systems will properly support any non-zero value in this field.
    //

    //TODO: WTF should I change this lol?!?!! This specification fucking sucks
    bootSector->RsvdSecCnt = 32;

    //
    // Number of file allocation tables can set to more if you want to
    // provide for redundancy in case sector containing the FAT becomes
    // corrupted
    //

    bootSector->BPB_NumFATs = 1;

    //
    // Legacy fields from FAT12 and FAT16
    //
   
    bootSector->BPB_RootEntCnt = 0;  
    bootSector->BPB_TotSec16 = 0;

    //
    // The value 0xF8 is the standard value for fixed non-removable media
    // devices as stated in the FAT specification
    //

    bootSector->BPB_Media = 0xF8;

    //
    // Legacy field from FAT12/FAT16
    //

    bootSector->BPB_FATSz16 = 0;

    //
    // Sectors per track for interrupt 0x13. This field is only relevant for
    // media that have a geometry (volume is broken down into tracks by
    // multiple heads and cylinders) and are visible on interrupt 0x13.
    // This field contains the “sectors per track” geometry value. 
    //

    bootSector->BPB_SecPerTrk = VFSExtension->DiskGeometry.SectorsPerTrack;

    //
    // Number of heads for interrupt 0x13. This field is relevant as
    // discussed earlier for BPB_SecPerTrk. This field contains the one
    // based “count of heads”. For example, on a 1.44 MB 3.5-inch floppy
    // drive this value is 2
    //

    bootSector->BPB_NumHeads = VFSExtension->DiskGeometry.TracksPerCylinder;

    //
    // Count of hidden sectors preceding the partition that contains this
    // FAT volume. This field is generally only relevant for media visible
    // on interrupt 0x13. This field should always be zero on media that
    // are not partitioned. Exactly what value is appropriate is operating
    // system specific. 
    //

    bootSector->BPB_HiddSec = 0;
    
    //
    // This field is the new 32-bit total count of sectors on the volume.
    // This count includes the count of all sectors in all four regions of the
    // volume. This field can be 0; if it is 0, then BPB_TotSec16 must be
    // non-zero. For FAT32 volumes, this field must be non-zero. For
    // FAT12/FAT16 volumes, this field contains the sector count if
    // BPB_TotSec16 is 0 (count is greater than or equal to 0x10000). 
    //
    
    bootSector->BPB_TotSec32 = VFSExtension->DiskSize / VFSExtension->DiskGeometry.BytesPerSector;

    //
    // This field is only defined for FAT32 media and does not exist on
    // FAT12 and FAT16 media. This field is the FAT32 32-bit count of
    // sectors occupied by ONE FAT. BPB_FATSz16 must be 0. 
    //
    
    //TODO: bootSector->BPB_FATSz32 = ;

    //
    // Calculate number of sectors required for FAT
    //

    fatEntries =
        (bootSector->bsSectors - bootSector->bsResSectors -
            bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
                bootSector->bsSecPerClus + 2;

    //
    // Choose between 12 and 16 bit FAT based on number of clusters we
    // need to map
    //

    if (fatEntries > 4087) {
        fatType =  16;
        fatSectorCnt = (fatEntries * 2 + 511) / 512;
        fatEntries   = fatEntries + fatSectorCnt;
        fatSectorCnt = (fatEntries * 2 + 511) / 512;
    }
    else {
        fatType =  12;
        fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
        fatEntries   = fatEntries + fatSectorCnt;
        fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
    }

    //
    // This field is only defined for FAT32 media and does not exist on
    // FAT12 and FAT16 media. Bits 0-3 -- Zero-based number of active FAT. 
    // Only valid if mirroring is disabled.
    //
    // Bits 4-6 -- Reserved.
    // Bit 7 -- 0 means the FAT is mirrored at runtime into all FATs.
    // -- 1 means only one FAT is active; it is the one referenced
    // in bits 0-3.
    // Bits 8-15 -- Reserved. 
    //

    //TODO:BPB_ExtFlags

    //
    // This field is only defined for FAT32 media and does not exist on
    // FAT12 and FAT16 media. High byte is major revision number.
    // Low byte is minor revision number. This is the version number of
    // the FAT32 volume. This supports the ability to extend the FAT32
    // media type in the future without worrying about old FAT32 drivers
    // mounting the volume. This document defines the version to 0:0. If
    // this field is non-zero, back-level Windows versions will not mount
    // the volume.
    //
    // NOTE: Disk utilities should respect this field and not operate on
    // volumes with a higher major or minor version number than that for
    // which they were designed. FAT32 file system drivers must check
    // this field and not mount the volume if it does not contain a version
    //

   bootSector->BPB_FSVeri = 0; 

   //
   // This field is only defined for FAT32 media and does not exist on
   // FAT12 and FAT16 media. This is set to the cluster number of the
   // first cluster of the root directory, usually 2 but not required to be 2.
   //
   // NOTE: Disk utilities that change the location of the root directory
   // should make every effort to place the first cluster of the root
   // directory in the first non-bad cluster on the drive (i.e., in cluster 2,
   // unless it’s marked bad). This is specified so that disk repair utilities
   // can easily find the root directory if this field accidentally gets
   // zeroed
   //

   //TODO:bootSector->BPB_RootClus 

   //
   // This field is only defined for FAT32 media and does not exist on
   // FAT12 and FAT16 media. Sector number of FSINFO structure in the
   // reserved area of the FAT32 volume. Usually 1.
   //
   // NOTE: There will be a copy of the FSINFO structure in BackupBoot,
   // but only the copy pointed to by this field will be kept up to date (i.e.,
   // both the primary and backup boot record will point to the same
   // FSINFO sector).
   //


   //TODO:bootSector->BPB_FSInfo 


   //
   // This field is only defined for FAT32 media and does not exist on
   // FAT12 and FAT16 media. 
   //
   // Set to 0 or 6.
   //
   // If non-zero, indicates the sector number in the
   // reserved area of the volume of a copy of the 
   // boot record.
   //

   //TODO:bootSector->BPB_BkBootSec = 6;

   //
   // This field is only defined for FAT32 media and does not exist on
   // FAT12 and FAT16 media. Reserved for future expansion. Code
   // that formats FAT32 volumes should always set all of the bytes of
   // this field to 0.
   //

   RtlZeroMemory(bootSector->BPB_Reserved, sizeof(bootSector->BPB_Reserved));

   //
   // Interrupt 0x13 drive number. Set value to 0x80 or 0x00
   //

   bootSector->BS_DrvNum = 0x80;

   //
   // Reserved. Set value to 0x0.
   //

   bootSector->BS_Reserved1 = 0;

   //
   // Extended boot signature. Set value to 0x29 if either of
   // the following two fields are non-zero.
   //
   // This is a signature byte that indicates that the
   // following three fields in the boot sector are present.
   //
   
   bootSector->BS_BootSig = 0x29;

   //
   // Volume serial number.
   //
   // This field, together with BS_VolLab, supports
   // volume tracking on removable media. These values
   // allow FAT file system drivers to detect that the wrong
   // disk is inserted in a removable drive.
   //
   // This ID should be generated by simply combining the
   // current date and time into a 32-bit value.
   //

   bootSector->BS_VolID = 0xC0FFFFEE;

   //
   // Volume label. This field matches the 11-byte volume
   // label recorded in the root directory.
   //
   // NOTE: FAT file system drivers must ensure that they
   // update this field when the volume label file in the root
   // directory has its name changed or created. The
   // setting for this field when there is no volume label is
   // the string “NO NAME ”.
   //

   bootSector->BS_VolLab[0] =  'N';
   bootSector->BS_VolLab[1] =  'O';
   bootSector->BS_VolLab[2] =  ' ';
   bootSector->BS_VolLab[3] =  'N';
   bootSector->BS_VolLab[4] =  'A';
   bootSector->BS_VolLab[5] =  'M';
   bootSector->BS_VolLab[6] =  'E';
   bootSector->BS_VolLab[7] =  ' ';
   bootSector->BS_VolLab[8] =  ' ';
   bootSector->BS_VolLab[9] =  ' ';
   bootSector->BS_VolLab[10] = ' ';

   //
   // Set to the string:”FAT32”
   //
   // NOTE: This string is informational only and does not
   // determine the FAT type.
   //
   
   bootSector->BS_FileSystemType[0] = 'F';
   bootSector->BS_FileSystemType[1] = 'A';
   bootSector->BS_FileSystemType[2] = 'T';
   bootSector->BS_FileSystemType[3] = '3';
   bootSector->BS_FileSystemType[4] = '2';
   bootSector->BS_FileSystemType[5] = ' ';
   bootSector->BS_FileSystemType[6] = ' ';
   bootSector->BS_FileSystemType[7] = ' ';

   //
   // Set to 0x00 
   //

   RtlZeroMemory(bootSector->BS_ReservedPadding, sizeof(bootSector->BS_ReservedPadding));

   //
   // Set to 0x55 (at byte offset 510) and 0xAA (at byte offset 511)
   //

   bootSector->BS_Signature[0] = 0x55;
   bootSector->BS_Signature[1] = 0xAA;
   
   return STATUS_SUCCESS;
   */
}

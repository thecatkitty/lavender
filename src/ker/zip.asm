%define ZIP_API
%include "err.inc"
%include "str.inc"
%include "uni.inc"
%include "zip.inc"


                cpu     8086

[bits 16]
section .text


                global  ZipLocateCentralDirectoryEnd
ZipLocateCentralDirectoryEnd:
                sub     si, ZIP_CDIR_END_HEADER_size
                ja      .Next
                ERR     ZIP_CDIR_END_NOT_FOUND
.Next:
                dec     si
                cmp     si, bx
                ERRC    ZIP_CDIR_END_NOT_FOUND

                cmp     word [si + ZIP_CDIR_END_HEADER.PkSignature], ZIP_PK_SIGN
                jne     .Next

                cmp     word [si + ZIP_CDIR_END_HEADER.HeaderSignature], ZIP_CDIR_END_SIGN
                jne     .Next

.Error:
.End:           ret


                global  ZipLocateFileHeader
ZipLocateFileHeader:
                mov     di, [si + ZIP_CDIR_END_HEADER.CentralDirectorySize + 2]
                test    di, di
                jz      .LocateInDirectory
                ERR     ZIP_CDIR_TOO_LARGE
.LocateInDirectory:
                mov     di, si
                sub     di, [si + ZIP_CDIR_END_HEADER.CentralDirectorySize]
.NextDirectoryEntry:
                cmp     word [di + ZIP_CDIR_FILE_HEADER.PkSignature], ZIP_PK_SIGN
                je      .CheckDirectorySignature
                ERR     ZIP_CDIR_INVALID
.CheckDirectorySignature:
                cmp     word [di + ZIP_CDIR_FILE_HEADER.HeaderSignature], ZIP_CDIR_END_SIGN
                je      .NotFound
                cmp     word [di + ZIP_CDIR_FILE_HEADER.HeaderSignature], ZIP_CDIR_FILE_SIGN
                je      .CheckName
                ERR     ZIP_CDIR_INVALID
.CheckName:
                cmp     byte [di + ZIP_CDIR_FILE_HEADER.Version + 1], ZIP_VERSION_FS_MSDOS
                je      .CaseInsensitive
                cmp     byte [di + ZIP_CDIR_FILE_HEADER.Version + 1], ZIP_VERSION_FS_NTFS
                je      .CaseInsensitive
                cmp     byte [di + ZIP_CDIR_FILE_HEADER.Version + 1], ZIP_VERSION_FS_VFAT
                jne     .CaseSensitive
.CaseInsensitive:
                cmp     word [di + ZIP_CDIR_FILE_HEADER.NameLength], cx
                jne     .CheckExtraFields
                push    si
                mov     si, di
                add     si, ZIP_CDIR_FILE_HEADER.Name
                call    UniCompareUtf8IgnoreCase
                pop     si
                jc      .Iterate
                jne     .Iterate
                jmp     .LocateLocalHeader
.CaseSensitive:
                cmp     word [di + ZIP_CDIR_FILE_HEADER.NameLength], cx
                jne     .Iterate
                push    si
                mov     si, di
                add     si, ZIP_CDIR_FILE_HEADER.Name
                call    StrCompareMemory
                pop     si
                je      .LocateLocalHeader
.CheckExtraFields:
                push    si
                push    cx
                mov     si, di
                add     si, ZIP_CDIR_FILE_HEADER_size
                add     si, word [di + ZIP_CDIR_FILE_HEADER.NameLength]
                mov     cx, si
                add     cx, word [di + ZIP_CDIR_FILE_HEADER.ExtraLength]
.NextExtraField:
                cmp     si, cx
                jae     .NotInExtraFields
                cmp     word [si + ZIP_EXTRA_FIELD_HEADER.Signature], ZIP_EXTRA_INFOZIP_UNICODE_PATH
                je      .InfozipUnicodePath
                add     si, word [si + ZIP_EXTRA_FIELD_HEADER.TotalSize]
                add     si, ZIP_EXTRA_FIELD_HEADER_size
                jmp     .NextExtraField
.InfozipUnicodePath:
                mov     cx, word [si + ZIP_EXTRA_FIELD_HEADER.TotalSize]
                sub     cx, (ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD.UnicodeName - ZIP_EXTRA_FIELD_HEADER_size)
                add     si, ZIP_EXTRA_INFOZIP_UNICODE_PATH_FIELD.UnicodeName
                call    UniCompareUtf8IgnoreCase
                jc      .NotInExtraFields
                jne     .NotInExtraFields
                pop     cx
                pop     si
                jmp     .LocateLocalHeader
.NotInExtraFields:
                pop     cx
                pop     si
.Iterate:
                push    cx
                mov     cx, ZIP_CDIR_FILE_HEADER_size
                add     cx, [di + ZIP_CDIR_FILE_HEADER.NameLength]
                add     cx, [di + ZIP_CDIR_FILE_HEADER.ExtraLength]
                add     cx, [di + ZIP_CDIR_FILE_HEADER.CommentLength]
                add     di, cx
                pop     cx
                jmp     .NextDirectoryEntry

.LocateLocalHeader:
                push    ax
                mov     ax, si
                sub     ax, [si + ZIP_CDIR_END_HEADER.CentralDirectoryOffset]
                sub     ax, [si + ZIP_CDIR_END_HEADER.CentralDirectorySize]
                add     ax, [di + ZIP_CDIR_FILE_HEADER.LocalHeaderOffset]
                mov     di, ax
                pop     ax
                cmp     word [di + ZIP_LOCAL_FILE_HEADER.PkSignature], ZIP_PK_SIGN
                je      .CheckLocalSignature
                ERR     ZIP_LOCAL_INVALID
.CheckLocalSignature:
                cmp     word [di + ZIP_LOCAL_FILE_HEADER.HeaderSignature], ZIP_LOCAL_FILE_SIGN
                je      .End
                ERR     ZIP_LOCAL_INVALID

.NotFound:
                ERR     ZIP_NOT_FOUND
.Error:
.End:           ret


                global  ZipLocateFileData
ZipLocateFileData:
                cmp     word [di + ZIP_LOCAL_FILE_HEADER.PkSignature], ZIP_PK_SIGN
                je      .CheckLocalSignature
                ERR     ZIP_LOCAL_INVALID
.CheckLocalSignature:
                cmp     word [di + ZIP_LOCAL_FILE_HEADER.HeaderSignature], ZIP_LOCAL_FILE_SIGN
                je      .CheckMethod
                ERR     ZIP_LOCAL_INVALID
.CheckMethod:
                cmp     word [di + ZIP_LOCAL_FILE_HEADER.Compression], ZIP_METHOD_STORE
                je      .CheckFlags
                ERR     ZIP_UNSUPPORTED_METHOD
.CheckFlags:
                test    word [di + ZIP_LOCAL_FILE_HEADER.Flags], ~ZIP_FLAGS_SUPPORTED
                jz      .LoadData
                ERR     ZIP_UNSUPPORTED_FLAGS
.LoadData:
                mov     bx, di
                add     bx, ZIP_LOCAL_FILE_HEADER_size
                add     bx, [di + ZIP_LOCAL_FILE_HEADER.NameLength]
                add     bx, [di + ZIP_LOCAL_FILE_HEADER.ExtraLength]
                jmp     .End

.Error:
.End:           ret

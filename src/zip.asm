%define ZIP_API
%include "err.inc"
%include "str.inc"
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
                cmp     word [di + ZIP_CDIR_FILE_HEADER.NameLength], cx
                jne     .Iterate
                push    bx
                push    si
                mov     si, di
                add     si, ZIP_CDIR_FILE_HEADER.Name
                call    StrCompareMemory
                pop     si
                pop     bx
                jne     .Iterate
                jmp     .LocateLocalHeader
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

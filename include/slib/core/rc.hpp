/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
#ifndef sLib_core_rc_hpp
#define sLib_core_rc_hpp

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <slib/core/def.hpp>
#include <slib/core/str.hpp>

#define SLIB_RC_OPERATION_ENTRIES \
    SLIB_RC_ENTRY_WITH_VAL(eOperationNone, 0, 0), \
    SLIB_RC_ENTRY(eAccessing, "accessing"), \
    SLIB_RC_ENTRY(eAllocating, "allocating"), \
    SLIB_RC_ENTRY(eAppending, "appending"), \
    SLIB_RC_ENTRY(eArithmetic, "performing arithmetic"), \
    SLIB_RC_ENTRY(eAttaching, "attaching"), \
    SLIB_RC_ENTRY(eAuthenticating, "authenticating"), \
    SLIB_RC_ENTRY(eAuthorizing, "authorizing"), \
    SLIB_RC_ENTRY(eBackingUp, "backing up"), \
    SLIB_RC_ENTRY(eBalancing, "balancing"), \
    SLIB_RC_ENTRY(eCaching, "caching"), \
    SLIB_RC_ENTRY(eCalling, "calling"), \
    SLIB_RC_ENTRY(eChecking, "checking"), \
    SLIB_RC_ENTRY(eCasting, "type-casting"), \
    SLIB_RC_ENTRY(eClassifying, "classifying"), \
    SLIB_RC_ENTRY(eCleaningUp, "cleaning up"), \
    SLIB_RC_ENTRY(eClearing, "clearing"), \
    SLIB_RC_ENTRY(eClosing, "closing"), \
    SLIB_RC_ENTRY(eCommitting, "committing"), \
    SLIB_RC_ENTRY(eComparing, "comparing"), \
    SLIB_RC_ENTRY(eCompiling, "compiling"), \
    SLIB_RC_ENTRY(eCompressing, "compressing"), \
    SLIB_RC_ENTRY(eConcatenating, "concatenating"), \
    SLIB_RC_ENTRY(eConnecting, "connecting"), \
    SLIB_RC_ENTRY(eConstructing, "constructing"), \
    SLIB_RC_ENTRY(eConverting, "converting"), \
    SLIB_RC_ENTRY(eCopying, "copying"), \
    SLIB_RC_ENTRY(eCreating, "creating"), \
    SLIB_RC_ENTRY(eCutting, "cutting"), \
    SLIB_RC_ENTRY(eDeallocating, "deallocating"), \
    SLIB_RC_ENTRY(eDecrypting, "decrypting"), \
    SLIB_RC_ENTRY(eDeleting, "deleting"), \
    SLIB_RC_ENTRY(eDestroying, "destroying"), \
    SLIB_RC_ENTRY(eDetaching, "detaching"), \
    SLIB_RC_ENTRY(eDisconnecting, "disconnecting"), \
    SLIB_RC_ENTRY(eEncrypting, "encrypting"), \
    SLIB_RC_ENTRY(eEscaping, "escaping"), \
    SLIB_RC_ENTRY(eEvaluating, "evaluating"), \
    SLIB_RC_ENTRY(eExecuting, "executing"), \
    SLIB_RC_ENTRY(eExpanding, "expanding"), \
    SLIB_RC_ENTRY(eFinding, "finding"), \
    SLIB_RC_ENTRY(eFlushing, "flushing"), \
    SLIB_RC_ENTRY(eFormatting, "formatting"), \
    SLIB_RC_ENTRY(eGrabbing, "grabbing"), \
    SLIB_RC_ENTRY(eHashing, "hashing"), \
    SLIB_RC_ENTRY(eIdentifying, "identifying"), \
    SLIB_RC_ENTRY(eIndexing, "indexing"), \
    SLIB_RC_ENTRY(eInitializing, "initializing"), \
    SLIB_RC_ENTRY(eInserting, "inserting"), \
    SLIB_RC_ENTRY(eInverting, "inverting"), \
    SLIB_RC_ENTRY(eJoining, "joining"), \
    SLIB_RC_ENTRY(eLaunching, "launching"), \
    SLIB_RC_ENTRY(eLinking, "linking"), \
    SLIB_RC_ENTRY(eLoading, "loading"), \
    SLIB_RC_ENTRY(eLocking, "locking"), \
    SLIB_RC_ENTRY(eLogging, "logging"), \
    SLIB_RC_ENTRY(eMapping, "mapping"), \
    SLIB_RC_ENTRY(eMoving, "moving"), \
    SLIB_RC_ENTRY(eOpening, "opening"), \
    SLIB_RC_ENTRY(ePacking, "packing"), \
    SLIB_RC_ENTRY(eParsing, "parsing"), \
    SLIB_RC_ENTRY(ePausing, "pausing"), \
    SLIB_RC_ENTRY(ePopping, "popping"), \
    SLIB_RC_ENTRY(ePositioning, "positioning"), \
    SLIB_RC_ENTRY(ePreparing, "preparing"), \
    SLIB_RC_ENTRY(ePrepending, "prepending"), \
    SLIB_RC_ENTRY(ePrinting, "printing"), \
    SLIB_RC_ENTRY(eProcessing, "processing"), \
    SLIB_RC_ENTRY(ePurging, "purging"), \
    SLIB_RC_ENTRY(ePushing, "pushing"), \
    SLIB_RC_ENTRY(eReading, "reading"), \
    SLIB_RC_ENTRY(eReceiving, "receiving"), \
    SLIB_RC_ENTRY(eRegistering, "registering"), \
    SLIB_RC_ENTRY(eReleasing, "releasing"), \
    SLIB_RC_ENTRY(eRenaming, "renaming"), \
    SLIB_RC_ENTRY(eRemoving, "removing"), \
    SLIB_RC_ENTRY(eResetting, "resetting"), \
    SLIB_RC_ENTRY(eResizing, "resizing"), \
    SLIB_RC_ENTRY(eResolving, "resolving"), \
    SLIB_RC_ENTRY(eRestarting, "restarting"), \
    SLIB_RC_ENTRY(eResuming, "resuming"), \
    SLIB_RC_ENTRY(eRetrying, "retrying"), \
    SLIB_RC_ENTRY(eReversing, "reversing"), \
    SLIB_RC_ENTRY(eReverting, "reverting"), \
    SLIB_RC_ENTRY(eRunning, "running"), \
    SLIB_RC_ENTRY(eSaving, "saving"), \
    SLIB_RC_ENTRY(eScanning, "scanning"), \
    SLIB_RC_ENTRY(eSearching, "searching"), \
    SLIB_RC_ENTRY(eSelecting, "selecting"), \
    SLIB_RC_ENTRY(eSending, "sending"), \
    SLIB_RC_ENTRY(eSerializing, "serializing"), \
    SLIB_RC_ENTRY(eSetting, "setting"), \
    SLIB_RC_ENTRY(eShrinking, "shrinking"), \
    SLIB_RC_ENTRY(eSignaling, "signaling"), \
    SLIB_RC_ENTRY(eSleeping, "sleeping"), \
    SLIB_RC_ENTRY(eSorting, "sorting"), \
    SLIB_RC_ENTRY(eSplitting, "splitting"), \
    SLIB_RC_ENTRY(eStarting, "starting"), \
    SLIB_RC_ENTRY(eStopping, "stopping"), \
    SLIB_RC_ENTRY(eTerminating, "terminating"), \
    SLIB_RC_ENTRY(eTesting, "testing"), \
    SLIB_RC_ENTRY(eTokenizing, "tokenizing"), \
    SLIB_RC_ENTRY(eUncompressing, "uncompressing"), \
    SLIB_RC_ENTRY(eUnescaping, "unescaping"), \
    SLIB_RC_ENTRY(eUnlinking, "unlinking"), \
    SLIB_RC_ENTRY(eUnlocking, "unlocking"), \
    SLIB_RC_ENTRY(eUnpacking, "unpacking"), \
    SLIB_RC_ENTRY(eUnregistering, "unregistering"), \
    SLIB_RC_ENTRY(eUpdating, "updating"), \
    SLIB_RC_ENTRY(eValidating, "validating"), \
    SLIB_RC_ENTRY(eVisiting, "visiting"), \
    SLIB_RC_ENTRY(eWaiting, "waiting"), \
    SLIB_RC_ENTRY(eWriting, "writing"), \
    SLIB_RC_ENTRY(eDownload, "downloading"), \
    SLIB_RC_ENTRY_WITH_VAL(eOperationOther, 255, "some operation on")

#define SLIB_RC_ENTITY_ENTRIES \
    SLIB_RC_ENTRY_WITH_VAL(eEntityNone, 0, 0), \
    SLIB_RC_ENTRY(eAddress, "address"), \
    SLIB_RC_ENTRY(eAlgorithm, "algorithm"), \
    SLIB_RC_ENTRY(eAnnotation, "annotation"), \
    SLIB_RC_ENTRY(eArchive, "archive"), \
    SLIB_RC_ENTRY(eArray, "array"), \
    SLIB_RC_ENTRY(eBlob, "blob"), \
    SLIB_RC_ENTRY(eBranch, "branch"), \
    SLIB_RC_ENTRY(eBuffer, "buffer"), \
    SLIB_RC_ENTRY(eByteOrder, "byte order"), \
    SLIB_RC_ENTRY(eCallback, "callback"), \
    SLIB_RC_ENTRY(eCategory, "category"), \
    SLIB_RC_ENTRY(eCell, "cell"), \
    SLIB_RC_ENTRY(eChain, "chain"), \
    SLIB_RC_ENTRY(eCharacter, "character"), \
    SLIB_RC_ENTRY(eChecksum, "checksum"), \
    SLIB_RC_ENTRY(eChild, "child"), \
    SLIB_RC_ENTRY(eClass, "class"), \
    SLIB_RC_ENTRY(eCode, "code"), \
    SLIB_RC_ENTRY(eColumn, "column"), \
    SLIB_RC_ENTRY(eCommand, "command"), \
    SLIB_RC_ENTRY(eCommandLine, "command line"), \
    SLIB_RC_ENTRY(eConfig, "configuration"), \
    SLIB_RC_ENTRY(eConnection, "connection"), \
    SLIB_RC_ENTRY(eConstraint, "constraint"), \
    SLIB_RC_ENTRY(eContext, "context"), \
    SLIB_RC_ENTRY(eCookie, "cookie"), \
    SLIB_RC_ENTRY(eCount, "count"), \
    SLIB_RC_ENTRY(eCSV, "CSV data"), \
    SLIB_RC_ENTRY(eCursor, "cursor"), \
    SLIB_RC_ENTRY(eData, "data"), \
    SLIB_RC_ENTRY(eDatabase, "database"), \
    SLIB_RC_ENTRY(eDescriptor, "descriptor"), \
    SLIB_RC_ENTRY(eDestination, "destination"), \
    SLIB_RC_ENTRY(eDictionary, "dictionary"), \
    SLIB_RC_ENTRY(eDirectory, "directory"), \
    SLIB_RC_ENTRY(eDiskSpace, "disk space"), \
    SLIB_RC_ENTRY(eDistance, "distance"), \
    SLIB_RC_ENTRY(eDocument, "document"), \
    SLIB_RC_ENTRY(eDomain, "domain"), \
    SLIB_RC_ENTRY(eElement, "element"), \
    SLIB_RC_ENTRY(eEmail, "email"), \
    SLIB_RC_ENTRY(eEncryption, "encryption"), \
    SLIB_RC_ENTRY(eEnd, "end"), \
    SLIB_RC_ENTRY(eEngine, "engine"), \
    SLIB_RC_ENTRY(eEnvironment, "environment"), \
    SLIB_RC_ENTRY(eExpression, "expression"), \
    SLIB_RC_ENTRY(eExtension, "extension"), \
    SLIB_RC_ENTRY(eFactory, "factory"), \
    SLIB_RC_ENTRY(eFASTA, "FASTA data"), \
    SLIB_RC_ENTRY(eField, "field"), \
    SLIB_RC_ENTRY(eFile, "file"), \
    SLIB_RC_ENTRY(eFileSystem, "filesystem"), \
    SLIB_RC_ENTRY(eFlag, "flag"), \
    SLIB_RC_ENTRY(eForm, "form"), \
    SLIB_RC_ENTRY(eFormat, "format"), \
    SLIB_RC_ENTRY(eFormula, "formula"), \
    SLIB_RC_ENTRY(eFrame, "frame"), \
    SLIB_RC_ENTRY(eFrequency, "frequency"), \
    SLIB_RC_ENTRY(eFunction, "function"), \
    SLIB_RC_ENTRY(eGroup, "group"), \
    SLIB_RC_ENTRY(eHandle, "handle"), \
    SLIB_RC_ENTRY(eHash, "hash"), \
    SLIB_RC_ENTRY(eHeader, "header"), \
    SLIB_RC_ENTRY(eHeap, "heap"), \
    SLIB_RC_ENTRY(eHierarchy, "hierarchy"), \
    SLIB_RC_ENTRY(eHost, "host"), \
    SLIB_RC_ENTRY(eID, "ID"), \
    SLIB_RC_ENTRY(eImage, "image"), \
    SLIB_RC_ENTRY(eIndex, "index"), \
    SLIB_RC_ENTRY(eInput, "input"), \
    SLIB_RC_ENTRY(eInterface, "interface"), \
    SLIB_RC_ENTRY(eIO, "I/O"), \
    SLIB_RC_ENTRY(eIon, "Ion"), \
    SLIB_RC_ENTRY(eItem, "item"), \
    SLIB_RC_ENTRY(eIterator, "iterator"), \
    SLIB_RC_ENTRY(eJob, "job"), \
    SLIB_RC_ENTRY(eJSON, "JSON data"), \
    SLIB_RC_ENTRY(eKey, "key"), \
    SLIB_RC_ENTRY(eLength, "length"), \
    SLIB_RC_ENTRY(eLetter, "letter"), \
    SLIB_RC_ENTRY(eLevel, "level"), \
    SLIB_RC_ENTRY(eLibrary, "library"), \
    SLIB_RC_ENTRY(eLimit, "limit"), \
    SLIB_RC_ENTRY(eLine, "line"), \
    SLIB_RC_ENTRY(eList, "list"), \
    SLIB_RC_ENTRY(eLocation, "location"), \
    SLIB_RC_ENTRY(eLock, "lock"), \
    SLIB_RC_ENTRY(eLog, "log"), \
    SLIB_RC_ENTRY(eMap, "map"), \
    SLIB_RC_ENTRY(eMatrix, "matrix"), \
    SLIB_RC_ENTRY(eMemMap, "memmap"), \
    SLIB_RC_ENTRY(eMemory, "memory"), \
    SLIB_RC_ENTRY(eMessage, "message"), \
    SLIB_RC_ENTRY(eMode, "mode"), \
    SLIB_RC_ENTRY(eModule, "module"), \
    SLIB_RC_ENTRY(eName, "name"), \
    SLIB_RC_ENTRY(eNode, "node"), \
    SLIB_RC_ENTRY(eNumber, "number"), \
    SLIB_RC_ENTRY(eObject, "object"), \
    SLIB_RC_ENTRY(eOffset, "offset"), \
    SLIB_RC_ENTRY(eOperation, "operation"), \
    SLIB_RC_ENTRY(eOrder, "order"), \
    SLIB_RC_ENTRY(eOutput, "output"), \
    SLIB_RC_ENTRY(eOwner, "owner"), \
    SLIB_RC_ENTRY(eParameter, "parameter"), \
    SLIB_RC_ENTRY(eParent, "parent"), \
    SLIB_RC_ENTRY(eParser, "parser"), \
    SLIB_RC_ENTRY(ePassword, "password"), \
    SLIB_RC_ENTRY(ePath, "path"), \
    SLIB_RC_ENTRY(ePattern, "pattern"), \
    SLIB_RC_ENTRY(ePermission, "permission"), \
    SLIB_RC_ENTRY(ePipe, "pipe"), \
    SLIB_RC_ENTRY(ePlugin, "plugin"), \
    SLIB_RC_ENTRY(ePointer, "pointer"), \
    SLIB_RC_ENTRY(ePool, "pool"), \
    SLIB_RC_ENTRY(ePosition, "position"), \
    SLIB_RC_ENTRY(eProcess, "process"), \
    SLIB_RC_ENTRY(eProperty, "property"), \
    SLIB_RC_ENTRY(eProtocol, "protocol"), \
    SLIB_RC_ENTRY(eQuery, "query"), \
    SLIB_RC_ENTRY(eQueue, "queue"), \
    SLIB_RC_ENTRY(eRange, "range"), \
    SLIB_RC_ENTRY(eReference, "reference"), \
    SLIB_RC_ENTRY(eReferenceCount, "reference count"), \
    SLIB_RC_ENTRY(eRequest, "request"), \
    SLIB_RC_ENTRY(eResource, "resource"), \
    SLIB_RC_ENTRY(eResponse, "response"), \
    SLIB_RC_ENTRY(eResult, "result"), \
    SLIB_RC_ENTRY(eRow, "row"), \
    SLIB_RC_ENTRY(eScope, "scope"), \
    SLIB_RC_ENTRY(eSeed, "seed"), \
    SLIB_RC_ENTRY(eSegment, "segment"), \
    SLIB_RC_ENTRY(eSelection, "selection"), \
    SLIB_RC_ENTRY(eSeparator, "separator"), \
    SLIB_RC_ENTRY(eSequence, "sequence"), \
    SLIB_RC_ENTRY(eService, "service"), \
    SLIB_RC_ENTRY(eSession, "session"), \
    SLIB_RC_ENTRY(eSignal, "signal"), \
    SLIB_RC_ENTRY(eSize, "size"), \
    SLIB_RC_ENTRY(eSocket, "socket"), \
    SLIB_RC_ENTRY(eSource, "source"), \
    SLIB_RC_ENTRY(eStack, "stack"), \
    SLIB_RC_ENTRY(eStart, "start"), \
    SLIB_RC_ENTRY(eString, "string"), \
    SLIB_RC_ENTRY(eStructure, "structure"), \
    SLIB_RC_ENTRY(eSubject, "subject"), \
    SLIB_RC_ENTRY(eSymbol, "symbol"), \
    SLIB_RC_ENTRY(eSymLink, "symlink"), \
    SLIB_RC_ENTRY(eTable, "table"), \
    SLIB_RC_ENTRY(eTarget, "target"), \
    SLIB_RC_ENTRY(eTemplate, "template"), \
    SLIB_RC_ENTRY(eText, "text"), \
    SLIB_RC_ENTRY(eThread, "thread"), \
    SLIB_RC_ENTRY(eTimeout, "timeout"), \
    SLIB_RC_ENTRY(eTimeStamp, "time stamp"), \
    SLIB_RC_ENTRY(eToken, "token"), \
    SLIB_RC_ENTRY(eTree, "tree"), \
    SLIB_RC_ENTRY(eType, "type"), \
    SLIB_RC_ENTRY(eURL, "URL"), \
    SLIB_RC_ENTRY(eUser, "user"), \
    SLIB_RC_ENTRY(eValue, "value"), \
    SLIB_RC_ENTRY(eVariable, "variable"), \
    SLIB_RC_ENTRY(eVersion, "version"), \
    SLIB_RC_ENTRY(eXML, "XML data"), \
    SLIB_RC_ENTRY(eNetwork, "network"), \
    SLIB_RC_ENTRY(eCertificate, "certificate"), \
    SLIB_RC_ENTRY(eServer, "server"), \
    SLIB_RC_ENTRY(eAuthentication, "authentication"), \
    SLIB_RC_ENTRY(eArguments, "arguments"), \
    SLIB_RC_ENTRY_WITH_VAL(eEntityOther, 255, "something")

#define SLIB_RC_STATE_ENTRIES \
    SLIB_RC_ENTRY_WITH_VAL(eStateOK, 0, 0), \
    SLIB_RC_ENTRY(eAmbiguous, "ambiguous"), \
    SLIB_RC_ENTRY(eBlocked, "blocked"), \
    SLIB_RC_ENTRY(eBusy, "busy"), \
    SLIB_RC_ENTRY(eCanceled, "canceled"), \
    SLIB_RC_ENTRY(eChanged, "changed"), \
    SLIB_RC_ENTRY(eCircularRef, "has a circular reference"), \
    SLIB_RC_ENTRY(eClosed, "closed"), \
    SLIB_RC_ENTRY(eCorrupt, "corrupt"), \
    SLIB_RC_ENTRY(eDeadlocked, "deadlocked"), \
    SLIB_RC_ENTRY(eDestroyed, "destroyed"), \
    SLIB_RC_ENTRY(eDetached, "detached"), \
    SLIB_RC_ENTRY(eDisabled, "disabled"), \
    SLIB_RC_ENTRY(eDown, "down"), \
    SLIB_RC_ENTRY(eDuplicate, "duplicate"), \
    SLIB_RC_ENTRY(eEnabled, "enabled"), \
    SLIB_RC_ENTRY(eEncrypted, "encrypted"), \
    SLIB_RC_ENTRY(eEmpty, "empty"), \
    SLIB_RC_ENTRY(eEqual, "equal"), \
    SLIB_RC_ENTRY(eExcessive, "excessive"), \
    SLIB_RC_ENTRY(eExhausted, "exhausted"), \
    SLIB_RC_ENTRY(eExists, "already exists"), \
    SLIB_RC_ENTRY(eExpired, "expired"), \
    SLIB_RC_ENTRY(eFailed, "failed"), \
    SLIB_RC_ENTRY(eIgnored, "ignored"), \
    SLIB_RC_ENTRY(eIncompatible, "incompatible"), \
    SLIB_RC_ENTRY(eIncomplete, "incomplete"), \
    SLIB_RC_ENTRY(eInconsistent, "inconsistent"), \
    SLIB_RC_ENTRY(eIncorrect, "incorrect"), \
    SLIB_RC_ENTRY(eInsecure, "potentially insecure"), \
    SLIB_RC_ENTRY(eInsufficient, "insufficient"), \
    SLIB_RC_ENTRY(eInterrupted, "interrupted"), \
    SLIB_RC_ENTRY(eInvalid, "invalid"), \
    SLIB_RC_ENTRY(eKilled, "killed"), \
    SLIB_RC_ENTRY(eLocked, "locked"), \
    SLIB_RC_ENTRY(eNotAuthenticated, "not authenticated"), \
    SLIB_RC_ENTRY(eNotAuthorized, "not authorized"), \
    SLIB_RC_ENTRY(eNotEqual, "not equal"), \
    SLIB_RC_ENTRY(eNotFinished, "not finished"), \
    SLIB_RC_ENTRY(eNotFound, "not found"), \
    SLIB_RC_ENTRY(eNotLocked, "not locked"), \
    SLIB_RC_ENTRY(eNotMatched, "not matched"), \
    SLIB_RC_ENTRY(eNotPermitted, "not permitted"), \
    SLIB_RC_ENTRY(eNotSorted, "not sorted"), \
    SLIB_RC_ENTRY(eNotSupported, "not supported"), \
    SLIB_RC_ENTRY(eNull, "is null"), \
    SLIB_RC_ENTRY(eObsolete, "obsolete"), \
    SLIB_RC_ENTRY(eOpen, "open"), \
    SLIB_RC_ENTRY(eOutOfRange, "out of range"), \
    SLIB_RC_ENTRY(ePaused, "paused"), \
    SLIB_RC_ENTRY(eProhibited, "prohibited"), \
    SLIB_RC_ENTRY(eReadOnly, "read-only"), \
    SLIB_RC_ENTRY(eRemoved, "removed"), \
    SLIB_RC_ENTRY(eStarted, "started"), \
    SLIB_RC_ENTRY(eStopped, "stopped"), \
    SLIB_RC_ENTRY(eTerminated, "terminated"), \
    SLIB_RC_ENTRY(eTimedOut, "timed out"), \
    SLIB_RC_ENTRY(eTooBig, "too big"), \
    SLIB_RC_ENTRY(eTooLong, "too long"), \
    SLIB_RC_ENTRY(eTooShort, "too short"), \
    SLIB_RC_ENTRY(eTooSmall, "too small"), \
    SLIB_RC_ENTRY(eUnavailable, "unavailable"), \
    SLIB_RC_ENTRY(eUndefined, "undefined"), \
    SLIB_RC_ENTRY(eUnexpected, "unexpected"), \
    SLIB_RC_ENTRY(eUninitialized, "uninitialized"), \
    SLIB_RC_ENTRY(eUnknown, "unknown"), \
    SLIB_RC_ENTRY(eUnreachable, "unreachable"), \
    SLIB_RC_ENTRY(eUnrecognized, "unrecognized"), \
    SLIB_RC_ENTRY(eViolated, "violated"), \
    SLIB_RC_ENTRY(eVirtual, "only virtual"), \
    SLIB_RC_ENTRY(eWriteOnly, "write-only"), \
    SLIB_RC_ENTRY(eWrongFormat, "in wrong format"), \
    SLIB_RC_ENTRY(eWrongOrder, "in wrong order"), \
    SLIB_RC_ENTRY(eWrongType, "of wrong type"), \
    SLIB_RC_ENTRY(eZero, "is zero"), \
    SLIB_RC_ENTRY(eNotProperlyPassed, "not properly passed"), \
    SLIB_RC_ENTRY_WITH_VAL(eStateOther, 255, "in some state")

#define SLIB_RC_ENTRY(id, description) id
#define SLIB_RC_ENTRY_WITH_VAL(id, val, description) id = val

namespace slib
{
    class sStr;

    struct sRC {
            enum EOperation { SLIB_RC_OPERATION_ENTRIES };
            enum EEntity { SLIB_RC_ENTITY_ENTRIES };
            enum EState { SLIB_RC_STATE_ENTRIES };

            union {
                    struct {
                            unsigned char op;
                            unsigned char op_target;
                            unsigned char bad_entity;
                            unsigned char state;
                    } parts;
                    uint32_t whole;
            } val;
            udx line;
            const char * file;
            sStr ctx;

            static const sRC zero;

            inline sRC()
            {
                val.whole = 0;
                line = 0;
                file = nullptr;
            }
#define RC(op_, op_target_, bad_entity_, state_, ...) sRC(op_, op_target_, bad_entity_, state_, __LINE__, __FILE__, ##__VA_ARGS__)
            inline sRC(EOperation op_, EEntity op_target_, EEntity bad_entity_, EState state_, udx line, const char * file, const char * context = nullptr, ...)
            {
                set(op_, op_target_, bad_entity_, state_, line, file);
                if( context && context[0] ) {
                    ctx.cut0cut();
                    sCallVarg(ctx.vprintf, context);
                }
            }

#define RCSET(rc, op_, op_target_, bad_entity_, state_, ...) rc.set(op_, op_target_, bad_entity_, state_, __LINE__, __FILE__, ##__VA_ARGS__)
#define RCSETP(rc, op_, op_target_, bad_entity_, state_, ...) rc->set(op_, op_target_, bad_entity_, state_, __LINE__, __FILE__, ##__VA_ARGS__)
            inline bool set(EOperation op_, EEntity op_target_, EEntity bad_entity_, EState state_, udx line, const char * file, const char * context = nullptr, ...)
            {
                val.parts.op = (unsigned char)op_;
                val.parts.op_target = (unsigned char)op_target_;
                val.parts.bad_entity = (unsigned char)bad_entity_;
                val.parts.state = (unsigned char)state_;
                this->line = line;
                const char * p = strrchr(file, '/');
                this->file = p ? p : file;
                if( context && context[0] ) {
                    ctx.cut0cut();
                    sCallVarg(ctx.vprintf, context);
                }
                return isSet();
            }

            inline sRC(const sRC & rhs)
            {
                operator=(rhs);
            }

            inline sRC & operator=(const sRC & rhs)
            {
                val = rhs.val;
                line = rhs.line;
                file = rhs.file;
                ctx.cutAddString(0, rhs.ctx.ptr(), rhs.ctx.length());
                return *this;
            }

            inline bool isSet() const
            {
                return val.whole;
            }

            inline bool isUnset() const
            {
                return !isSet();
            }

            inline operator bool() const
            {
                return isSet();
            }

            static const char * operation2string(EOperation op, bool asEnumName = false);
            static const char * entity2string(EEntity entity, bool asEnumName = false);
            static const char * state2string(EState state, bool asEnumName = false);
            const char * print(sStr * out = 0, bool asEnumNames = false) const;
    };
};

#endif

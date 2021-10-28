
/*************************************************************************
   LAB 1                                                                

    Edit this file ONLY!

*************************************************************************/



#include "dns.h"
#include "hashtable.h"
#define HASHTABLE_SIZE 13000
#define BUFFER_SIZE 1024
#define MAX_DOMAIN_SIZE 128

typedef struct {
    HashtableHandle lookup_table;
} DNSObject;

typedef struct {
    unsigned int ip;
    char domain[MAX_DOMAIN_SIZE];
    BOOL has_backup;
    size_t backup_length;
    int current_value;
    int num;
} Parser;


DNSHandle InitDNS( )
{
    DNSObject* dns = (DNSObject*)malloc(sizeof(DNSObject));
    if (dns == NULL) {
        return INVALID_DNS_HANDLE;
    }

    dns->lookup_table = HashCreate(HASHTABLE_SIZE);
    if (dns->lookup_table == NULL) {
        free(dns);
        return INVALID_DNS_HANDLE;
    }

    return (DNSHandle)dns;
}

void ParseFile(Parser* self, Hashtable* lookup, char* buffer) {
    char c;
    int num = self->num;
    BOOL name_started = FALSE;
    char* name_begin;
    size_t cur_length = 0;


    while ((c = *(buffer++)) != '\0') {

        if (self->current_value < 4) {
            if ('0' <= c && c <= '9') {
                num *= 10;
                num += c - '0';
            }
            else if (c == '.' || c == ' ') {
                self->ip = (self->ip << 8) + num;
                ++self->current_value;
                num = 0;
            }
        }

        else {
            if (c == '\r' || c == ' ') {
                continue;
            }
            else if (c == '\n') {
                // Set null-terminating char instead of \n to make strcat/strcpy work properly
                *(buffer - 1) = '\0';
                if (self->has_backup) {
                    if (name_started)
                        strcat_s(self->domain, MAX_DOMAIN_SIZE, name_begin);

                    HashInsertLength(lookup, self->domain, self->backup_length + cur_length, self->ip);
                    self->has_backup = FALSE;
                    self->backup_length = 0;
                }
                else {
                    HashInsertLength(lookup, name_begin, cur_length, self->ip);
                }
                *(buffer - 1) = '\n';
                name_started = FALSE;
                cur_length = 0;
                self->ip = 0;
                self->current_value = 0;
            }
            else {
                if (!name_started) {
                    name_begin = buffer - 1;
                    name_started = TRUE;
                }
                ++cur_length;
            }
        }
    }

    //Backup - carry current domain and num to the next function call in parser object
    if (name_started) {
        strcpy_s(self->domain, MAX_DOMAIN_SIZE, name_begin);
        self->has_backup = TRUE;
        self->backup_length = cur_length;
    }
    self->num = num;
}

void LoadHostsFile( DNSHandle hDNS, const char* hostsFilePath )
{
    DNSObject* dns = (DNSObject*)hDNS;
    FILE* fInput;
    fopen_s(&fInput, hostsFilePath, "r");
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->ip = 0;
    parser->current_value = 0;
    parser->has_backup = FALSE;
    parser->num = 0;

    char buffer[BUFFER_SIZE];
    buffer[BUFFER_SIZE - 1] = '\0';
    unsigned int read_bytes = 0;
    while((read_bytes = fread(buffer, 1, BUFFER_SIZE - 1, fInput)) > 0) {
        buffer[read_bytes] = '\0';
        ParseFile(parser, dns->lookup_table, buffer);
    }
    fclose(fInput);
}

IPADDRESS DnsLookUp( DNSHandle hDNS, const char* hostName )
{
    DNSObject* dns = (DNSObject*)hDNS;
    value_type value;
    if (HashFind(dns->lookup_table, hostName, &value)) {
        return value;
    }
    return INVALID_IP_ADDRESS;
}

double GetAverageLookupSize(DNSHandle hDNS) {
    DNSObject* dns = (DNSObject*)hDNS;
    return GetAverageSize(dns->lookup_table);
}

size_t GetMaximumLookupSize(DNSHandle hDNS) {
    DNSObject* dns = (DNSObject*)hDNS;
    return GetMaximumSize(dns->lookup_table);
}

void ShutdownDNS( DNSHandle hDNS )
{
    DNSObject* dns = (DNSObject*)hDNS;
    HashRelease(dns->lookup_table);
    free(dns);
}

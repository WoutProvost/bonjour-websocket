#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H

enum MessageType
{
	NONE,	// Used when the 'type' field is absent
	ALL,
	ADD_OR_UPDATE,
	REMOVE,
	REFRESH	// Client can manually ask for a refresh
};

#endif
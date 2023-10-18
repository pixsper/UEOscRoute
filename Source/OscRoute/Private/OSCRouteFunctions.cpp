// Fill out your copyright notice in the Description page of Project Settings.


#include "OSCRouteFunctions.h"

FOSCRouteConfig::FOSCRouteConfig()
{
	Id = FGuid::NewGuid();
}

FOSCMessage& UOSCRouteFunctions::Switch(FOSCMessage& Message, FOSCRoute& Route)
{
	return Message;
}

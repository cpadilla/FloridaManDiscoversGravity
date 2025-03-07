// Copyright 2017-2018 David Romanski (Socke). All Rights Reserved.

#include "SocketClientBPLibrary.h"
#include "SocketClient.h"

//USocketClientBPLibrary* USocketClientBPLibrary::socketClientBPLibrary;

USocketClientBPLibrary::USocketClientBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer){

	//socketClientBPLibrary = this;
	/*if (USocketClientHandler::socketClientHandler->getSocketClientTarget() == nullptr) {
		USocketClientHandler::socketClientHandler->addTCPClientToMap("0.0.0.0",0,this);
	 }*/

	//Delegates
	onsocketClientConnectionEventDelegate.AddDynamic(this, &USocketClientBPLibrary::socketClientConnectionEventDelegate);
	onreceiveTCPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveTCPMessageEventDelegate);
	onreceiveUDPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveUDPMessageEventDelegate);
}

/*Delegate functions*/
void USocketClientBPLibrary::socketClientConnectionEventDelegate(const bool success, const FString message) {}
void USocketClientBPLibrary::receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray) {}
void USocketClientBPLibrary::receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString IP, const int32 Port) {}


FString USocketClientBPLibrary::getLocalIP() {
	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);

	if (localIp->IsValid()) {
		return localIp->ToString(false);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Could not detect the local IP."));
	}
	return "127.0.0.1";
}


USocketClientBPLibrary::~USocketClientBPLibrary() {
	closeSocketClientConnection();
}

//USocketClientBPLibrary * USocketClientBPLibrary::getSocketClientTarget(){
//	return socketClientBPLibrary;
//}

void USocketClientBPLibrary::socketClientSendTCPMessage(FString message, TArray<uint8> byteArray, bool addLineBreak){
	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}
	
	new FSendDataToServerThread(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTarget(), "0.0.0.0", 0);
}

void USocketClientBPLibrary::socketClientSendTCPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak) {
	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	ip = USocketClientHandler::resolveDomain(ip);

	new FSendDataToServerThread(message, byteArray, USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(ip, port), ip, port);
}



void USocketClientBPLibrary::connectSocketClient(FString domain, int32 port) {
	/*serverDomain	= domain;
	serverPort		= port;*/

	UPROPERTY()
	USocketClientBPLibrary* socketClientBPLibrary = NewObject<USocketClientBPLibrary>(USocketClientBPLibrary::StaticClass());
	FString ip = USocketClientHandler::resolveDomain(domain);
	socketClientBPLibrary->setServerIP(ip);
	socketClientBPLibrary->setServerPort(port);
	socketClientBPLibrary->AddToRoot();

	USocketClientBPLibrary* oldClient = USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(ip, port);

	USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(ip, port, socketClientBPLibrary);
	//USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(domain, port, socketClientBPLibrary);

	//connect to server
	new FServerConnectionThread(socketClientBPLibrary, oldClient, domain);
}

void USocketClientBPLibrary::socketClientSendUDPMessage(FString domain, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak) {

	USocketClientBPLibrary* socketClientBPLibrary = USocketClientHandler::getSocketClientTarget();
	//if (socketClientBPLibrary->getUDPSocket() == nullptr) {
		socketClientBPLibrary->setServerIP(USocketClientHandler::resolveDomain(domain));
		socketClientBPLibrary->setServerPort(port);
	//}

	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}
	if (serverUDPConnectionThread == nullptr || serverUDPConnectionThread->isRun() == false) {
		serverUDPConnectionThread = new FServerUDPConnectionThread(this);
		FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		//set message
		serverUDPConnectionThread->setMessage(message, byteArray);
		//init thread. seted message is send after this line.
		serverUDPConnectionThread->setThread(FRunnableThread::Create(serverUDPConnectionThread, *threadName, 0, EThreadPriority::TPri_Normal));
	}
	else {
		//reuse old thread to send message. is much faster than each time a new thread
		serverUDPConnectionThread->sendMessage(message, byteArray);
	}

}

void USocketClientBPLibrary::socketClientInitUDPReceiver(FString domain, int32 port) {
	USocketClientBPLibrary* socketClientBPLibrary = USocketClientHandler::getSocketClientTarget();
	socketClientBPLibrary->setServerIP(USocketClientHandler::resolveDomain(domain));
	socketClientBPLibrary->setServerPort(port);
	FString message = "";
	TArray<uint8> byteArray;
	if (serverUDPConnectionThread == nullptr || serverUDPConnectionThread->isRun() == false) {
		serverUDPConnectionThread = new FServerUDPConnectionThread(this);
		FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		//serverUDPConnectionThread->setMessage(message, byteArray, socketClientBPLibrary);
		serverUDPConnectionThread->setThread(FRunnableThread::Create(serverUDPConnectionThread, *threadName, 0, EThreadPriority::TPri_Normal));
	}
	//else {
	//	serverUDPConnectionThread->sendMessage(message, byteArray, socketClientBPLibrary);
	//}
}

void USocketClientBPLibrary::UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt){

	TSharedPtr<FInternetAddr> peerAddr = EndPt.ToInternetAddr();
	FString ip = peerAddr->ToString(false);
	int32 port = peerAddr->GetPort();
	//UE_LOG(LogTemp, Error, TEXT("peerAddr:%s  port:%i"), *peerAddr->ToString(false), peerAddr->GetPort());

	char* Data = (char*)ArrayReaderPtr->GetData();
	Data[ArrayReaderPtr->Num()] = '\0';
	FString recvMessage = FString(UTF8_TO_TCHAR(Data));

	TArray<uint8> byteArray;
	byteArray.Append(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());

	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [recvMessage, byteArray, ip, port]() {
		USocketClientHandler::socketClientHandler->getSocketClientTarget()->onreceiveUDPMessageEventDelegate.Broadcast(recvMessage, byteArray, ip, port);
	});

}


void USocketClientBPLibrary::closeSocketClientConnection() {
	//tcp
	run = false;
	//udp
	if (serverUDPConnectionThread != nullptr) {
		serverUDPConnectionThread->stopThread();
		serverUDPConnectionThread = nullptr;
	}
	FSocket* socket = getUDPSocket();
	if (socket != nullptr && socket != NULL) {
		socket->Close();
		socket = NULL;
	}
	FUdpSocketReceiver* udpSR = getUDPSocketReceiver();
	if (udpSR != nullptr && udpSR != NULL) {
		udpSR->Stop();
		udpSR->Exit();
		udpSR = NULL;
	}

	setUDPSocket(NULL, NULL);
}


FString USocketClientBPLibrary::getServerIP() {
	return serverIP;
}

int32 USocketClientBPLibrary::getServerPort() {
	return serverPort;
}

void USocketClientBPLibrary::setServerIP(FString ip){
	serverIP = ip;
}

void USocketClientBPLibrary::setServerPort(int32 port){
	serverPort = port;
}

USocketClientBPLibrary * USocketClientBPLibrary::getSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port){
	return USocketClientHandler::socketClientHandler->getSocketClientTargetByIP_AndPortInternal(IP, Port);
}

void USocketClientBPLibrary::removeSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port) {
	USocketClientHandler::socketClientHandler->removeSocketClientTargetByIP_AndPortInternal(IP, Port);
}
void USocketClientBPLibrary::addSocketClientTargetFromSocketCllientHandler(FString IP, int32 Port, USocketClientBPLibrary* target){
	USocketClientHandler::socketClientHandler->addTCPClientToMap(IP,Port,target);
}


bool USocketClientBPLibrary::shouldRun() {
	return run;
}

void USocketClientBPLibrary::setRun(bool runP) {
	run = runP;
}

void USocketClientBPLibrary::setTCPSocket(FSocket * socketP) {
	socketTCP = socketP;
}

FSocket* USocketClientBPLibrary::getTCPSocket(){
	return socketTCP;
}

void USocketClientBPLibrary::setUDPSocket(FSocket * socketP, FUdpSocketReceiver* udpSocketReceiverP) {
	socketUDP = socketP;
	udpSocketReceiver = udpSocketReceiverP;
}

FSocket* USocketClientBPLibrary::getUDPSocket() {
	return socketUDP;
}

FUdpSocketReceiver* USocketClientBPLibrary::getUDPSocketReceiver() {
	return udpSocketReceiver;
}
//FString USocketClientBPLibrary::getUDPIP() {
//	return serverIPUDP;
//}
//
//int32 USocketClientBPLibrary::getUDPPort() {
//	return serverPortUDP;
//}
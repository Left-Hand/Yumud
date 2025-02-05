#pragma once

// package com.gcdc.canopen;

// import com.gcdc.can.CanMessage;
// import com.gcdc.can.Driver;
// import com.gcdc.can.CanMessageConsumer;
// import java.util.TimerTask;
// import java.util.Timer;
// import java.util.concurrent.atomic.*;
// import java.util.ArrayList;
// import java.util.Iterator;
// import java.util.List;

#include "Protocol.hpp"

namespace ymd::canopen{

class CanOpenNode{
public:
    int getNodeId();
    void toOperationalState();
    void toStoppedState();
    void toPreoperationalState();
};

// class CanOpenNode {
// 	static final int STATE_INIT  = 0x00;
// 	static final int STATE_DISCONNECTED    = 0x01;
// 	static final int STATE_CONNECTING      = 0x02;
// 	static final int STATE_PREPARING       = 0x02;
// 	static final int STATE_STOPPED         = 0x04;
// 	static final int STATE_OPERATIONAL     = 0x05;
// 	static final int STATE_PREOPERATIONAL = 0x7F;
// 	static final int STATE_UNKNOWN   = 0x0F;

// 	private Driver busDriver;
// 	private boolean debug = false;

// 	private Protocol nmt = null;
// 	private Sdo sdo = null;
// 	private Pdo pdo = null;
// 	private Protocol sync = null;
// 	private Protocol time = null;
// 	private Protocol emcy = null;

// 	private Protocol heartbeat = null;
// 	private Protocol lss = null;
// 	private int canOpenState = STATE_UNKNOWN;
// 	private int nodeId;

// 	ObjectDictionary objdict  = null; //DefaultOD.create(0x20);

// 	List<CanOpenListener> eventListeners;

// 	private void debugPrint(String smsg)
// 	{
// 		if(debug)
// 			System.out.println(smsg);
// 	}


// 	public CanOpenNode( Driver driver, ObjectDictionary od, int nodeId,  boolean DEBUG )
// 	{
// 		if( DEBUG )
// 		{
// 			debug = true;
// 		}
// 		debugPrint( "CanOpen Init" );

// 		if(od != null)
// 		{
// 			objdict = od;
// 		}

// 		eventListeners = new ArrayList<CanOpenListener>();

// 		setNodeId(nodeId);

// 		busDriver = driver;
// 		nmt = new Nmt(busDriver, false, this, objdict);
// 		toInitializationState();

// 	}





// 	public void start() throws Exception
// 	{
// 		if(canOpenState != STATE_INIT)
// 			return;
// 		busDriver.setMessageConsumer(this);
// 		busDriver.startTransfer();
// 		try
// 		{
// 			nmt.start();
// 		}
// 		catch(java.io.IOException e)
// 		{
// 			System.out.println(e);
// 		}

// 		toPreoperationalState();
// System.out.println("CanOpen.class WARNING: moving to operational state without master request, (not consistant with CanOpen specification)");
// 		toOperationalState();
// 	}

// 	// allow overiding application to acces the Object Dictionay
// 	public ObjectDictionary getObjDict()
// 	{
// 		return(objdict);
// 	}


// 	public void addEventListener(CanOpenListener coListener)
// 	{
// 		eventListeners.add(coListener);
// 	}


// 	public void removeEventListener(CanOpenListener coListener)
// 	{
// 		eventListeners.remove(coListener);
// 	}


// 	private void notifyListeners()
// 	{
// 		Iterator<CanOpenListener> coli = eventListeners.iterator();
// 		while(coli.hasNext())
// 		{
// 			CanOpenListener coListener = coli.next();
// 			coListener.onEvent(this, canOpenState);
// 		}
// 	}


// 	public void addNmtListener(CanOpenListener coListener)
// 	{
// 		nmt.addListener(coListener);
// 	}
// 	public void removeNmtListener(CanOpenListener coListener)
// 	{
// 		nmt.removeListener(coListener);
// 	}

// 	public void addSdoListener(CanOpenListener coListener)
// 	{
// 		sdo.addListener(coListener);
// 	}
// 	public void removeSdoListener(CanOpenListener coListener)
// 	{
// 		sdo.removeListener(coListener);
// 	}

// 	public void addPdoListener(CanOpenListener coListener)
// 	{
// 		pdo.addListener(coListener);
// 	}
// 	public void removePdoListener(CanOpenListener coListener)
// 	{
// 		pdo.removeListener(coListener);
// 	}

// 	public void addSyncListener(CanOpenListener coListener)
// 	{
// 		sync.addListener(coListener);
// 	}
// 	public void removeSyncListener(CanOpenListener coListener)
// 	{
// 		sync.removeListener(coListener);
// 	}

// 	public void addTimeListener(CanOpenListener coListener)
// 	{
// 		time.addListener(coListener);
// 	}
// 	public void removeTimeListener(CanOpenListener coListener)
// 	{
// 		time.removeListener(coListener);
// 	}

// 	public void addEmcyListener(CanOpenListener coListener)
// 	{
// 		emcy.addListener(coListener);
// 	}
// 	public void removeEmcyListener(CanOpenListener coListener)
// 	{
// 		emcy.removeListener(coListener);
// 	}

// 	public void addHeartbeatListener(CanOpenListener coListener)
// 	{
// 		heartbeat.addListener(coListener);
// 	}
// 	public void removeHeartbeatListener(CanOpenListener coListener)
// 	{
// 		heartbeat.removeListener(coListener);
// 	}

// 	public void addLssListener(CanOpenListener coListener)
// 	{
// 		lss.addListener(coListener);
// 	}
// 	public void removeLssListener(CanOpenListener coListener)
// 	{
// 		lss.removeListener(coListener);
// 	}



// 	public void processMessage(CanMessage msg)
// 	{
// 		if(msg == null)
// 		{
// 			debugPrint("null message recevied");
// 			return;
// 		}

// //		if(debug)
// //			msg.dump();


// 		int msgType = msg.id>>7;
// 		switch(msgType)
// 		{
// 		case Protocol.SYNC:            /* can be a SYNC or a EMCY message */
// 			if(msg.id == 0x080)
// 			{       // SYNC
// //				debugPrint("pmSYNC");
// 				sync.processMessage(msg);
// 			}
// 			else
// 			{       //EMCY
// 				debugPrint("pmEMCY");
// 				if(emcy.processMessage(msg) == true)
// 				{
// 					emcy.notifyListeners(msg);
// 				}
// 			}
// 		break;
// 		case Protocol.TIME_STAMP:
// 			debugPrint("pmTIME_STAMP");
// 			if(time.processMessage(msg) == true )
// 			{
// 				time.notifyListeners(msg);
// 			}
// 		break;
// 		case Protocol.PDO1tx:
// 		case Protocol.PDO1rx:
// 		case Protocol.PDO2tx:
// 		case Protocol.PDO2rx:
// 		case Protocol.PDO3tx:
// 		case Protocol.PDO3rx:
// 		case Protocol.PDO4tx:
// 		case Protocol.PDO4rx:
// //			debugPrint("pmPDO");
// 			pdo.processMessage(msg);
// 		break;
// 		case Protocol.SDOtx:
// 		case Protocol.SDOrx:
// //			debugPrint("pmSDO");
// 			sdo.processMessage(msg);
// 		break;
// 		case Protocol.NODE_GUARD:
// //			debugPrint("pmHeartBeat");
// 			heartbeat.processMessage(msg);
// 		break;
// 		case Protocol.NMT:
// 			debugPrint("pmNMT");
// 			nmt.processMessage(msg);
// 		break;
// 		case Protocol.LSS:
// 			debugPrint("pmLSS");
// 			if(lss.processMessage(msg) == true)
// 			{
// 				lss.notifyListeners(msg);
// 			}
// 		break;
// 		default:
// 			debugPrint("pmUNKNOWN canopen message type");
// 			msg.dump();
// 		break;
// 		}
// 	}


// 	int getCanOpenState()
// 	{
// 		return(canOpenState);
// 	}


// 	// the four basic states of the CAN-open protocol
// 	void toInitializationState()
// 	{
// 		debugPrint("toInitializationState");
// 		if(sdo != null)
// 		{
// 			sdo.stop();
// 			sdo = null;
// 		}

// 		if(pdo != null)
// 		{
// 			pdo.stop();
// 			pdo = null;
// 		}

// 		if(sync != null)
// 		{
// 			sync.stop();
// 			sync = null;
// 		}

// 		if(time != null)
// 		{
// 			time.stop();
// 			time = null;
// 		}

// 		if(emcy != null)
// 		{
// 			emcy.stop();
// 			emcy = null;
// 		}

// 		if(heartbeat != null)
// 		{
// 			heartbeat.stop();
// 			heartbeat = null;
// 		}

// 		if(lss != null)
// 		{
// 			lss.stop();
// 			lss = null;
// 		}

// 		System.gc();  //Runs the garbage collector to flush recently disposed objects

// 		try
// 		{
// 			sdo = new Sdo(busDriver, debug, objdict);
// 			pdo = new Pdo(busDriver, debug, objdict);
// 			heartbeat = new Heartbeat(busDriver, debug, objdict, this);
// 			sync = new Sync(busDriver, debug, pdo, objdict);
// 		}
// 		catch(Exception e)
// 		{
// 			System.out.println(""+e);
// 			System.exit(0);
// 		}

// 		time = new Protocol(busDriver, debug, "TIME_STAMP", objdict);
// 		emcy = new Protocol(busDriver, debug, "EMCY", objdict);
// //		heartbeat = new Protocol(busDriver, debug, "GUARD", objdict);
// 		lss = new Protocol(busDriver, debug, "LSS", objdict);

// 		AtomicInteger a1 = new AtomicInteger(0x700);
// 		heartbeat.addCobId(a1);
// 		canOpenState = STATE_INIT;
// 		notifyListeners();
// 	}


// 	void toPreoperationalState()
// 	{
// 	debugPrint("toPreop");
// 		try
// 		{
// //			nmt.stop();
// 			sdo.start();
// 			emcy.start();
// 			sync.start();
// //			heartbeat.start();
// 			pdo.stop();
// 			lss.start();
// 		}
// 		catch(java.io.IOException e)
// 		{
// 			System.out.println(e);
// 		}
// 		canOpenState =STATE_PREOPERATIONAL;
// 		notifyListeners();
// 	}


// 	void toOperationalState()
// 	{
// 		debugPrint("toOperational");
// 		try
// 		{
// //			nmt.stop();
// 			sdo.start();
// 			emcy.start();
// 			sync.start();
// 			heartbeat.start();
// 			pdo.start();
// 			lss.stop();
// 		}
// 		catch(java.io.IOException e)
// 		{
// 			System.out.println(e);
// 		}
// 		canOpenState = STATE_OPERATIONAL;
// 		notifyListeners();
// 	}


// 	void toStoppedState()
// 	{
// 		debugPrint("toStopped");
// 		try
// 		{
// //			nmt.stop();
// 			sdo.stop();
// 			emcy.stop();
// 			sync.stop();
// 			heartbeat.start();
// 			pdo.stop();
// 			lss.start();
// 		}
// 		catch(java.io.IOException e)
// 		{
// 			System.out.println(e);
// 		}
// 		canOpenState = STATE_STOPPED;
// 		notifyListeners();
// 	}


// 	private void setPdoCobId(int index, int cobId) throws Exception
// 	{
// 		OdEntry oe = objdict.getEntry(index);
// 		SubEntry se = oe.getSub(0);
// 		int len = se.getInt();
// //		debugPrint("len:"+len);
// 		{
// 			se = oe.getSub(1);
// 			int currCobId = se.getInt();
// //			debugPrint("curr cob-id: "+currCobId);
// 			se.set( cobId);
// 		}
// //		debugPrint(oe.toIniString());
// 	}


// 	void setNodeId(int nodeId)
// 	{
// 		if((nodeId<=0) || (nodeId>127))
// 		{
// 			System.out.println("Invalid NodeID "+nodeId);
// 			return;
// 		}

// 		try
// 		{
// 			{
// 				// Set SDO cob ids
// 				OdEntry oe = objdict.getEntry(0x1200);
// 				SubEntry se = oe.getSub(0);
// 				int len = se.getInt();
// //				debugPrint("len:"+len);

// 				se = oe.getSub(1);
// 				int currCobId = se.getInt();
// //				debugPrint("curr cob-id: "+currCobId);
// 				se.setIgnorePermissions(0x0600 | nodeId);

// 				se = oe.getSub(2);
// 				currCobId = se.getInt();
// //				debugPrint("curr cob-id: "+currCobId);
// 				se.setIgnorePermissions(0x0580 | nodeId);
// 			}
// //			debugPrint(oe.toIniString());

// 			// RPDO parameters
// //			setPdoCobId(0x1400, 0x200 | nodeId);
// //			setPdoCobId(0x1401, 0x300 | nodeId);
// //			setPdoCobId(0x1402, 0x400 | nodeId);
// //			setPdoCobId(0x1403, 0x500 | nodeId);

// 			setPdoCobId(0x1800, 0x180 | nodeId);
// 			setPdoCobId(0x1801, 0x280 | nodeId);
// 			setPdoCobId(0x1802, 0x380 | nodeId);
// 			setPdoCobId(0x1803, 0x480 | nodeId);

// //			cobIdEmcy = nodeId + 0x80;
// 			this.nodeId = nodeId;
// 		}
// 		catch(Exception e)
// 		{
// 			System.out.println(e);
// 		}
// 	}


// 	int getNodeId()
// 	{
// 		return(nodeId);
// 	}
// };

}
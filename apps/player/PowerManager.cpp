#include "player.h"

namespace mango
{

	int time[]={30,60,2*60,5*60,10*60};
	int len = 5;
	int power[]={60*5,60*10,60*20,60*30,-1};
	int plen = 4;
	
	PowerManager::PowerManager(){
		isSleep = 0;
		
		initAutoSleepTime();

		gSettingProvider.query(SETTING_AUTOPOWEROFF_ID,&poweroffTime);
		
		atuoCount = 0;
		atuoPoweroffCount = 0;
		setPowerState(0);
	}
	void PowerManager::initAutoSleepTime(){
		int i;
		int num = 3;
		
		for(i=0;i<num;i++){
			gSettingProvider.query(SETTING_AUTOSLEEP_ID,&sleepTime);
			if(getAutoSleepTime()>0)
				break;
		}
		
		if(num>=3)
			sleepTime = time[0];

		log_i("setPowerState sleepTime=%d.",sleepTime);
	}
	void PowerManager::setAutoSleepTime(int index){
		sleepTime = time[index];
		gSettingProvider.update(SETTING_AUTOSLEEP_ID,sleepTime);
	}
	void PowerManager::setAutoPoweroffTime(int index){
		if(index>=0&&index<=4){
			poweroffTime = power[index];
			gSettingProvider.update(SETTING_AUTOPOWEROFF_ID,poweroffTime);
			if(poweroffTime<0){
				gPlayer.setBootWakeLock(0);
			}else{
				gPlayer.setBootWakeLock(1);
			}
		}
	}

	void PowerManager::PowerManagerCount(){
		atuoCount++;
		if(atuoCount>=sleepTime&&(!isSleep)){
			log_i("setPowerState autosleep time out.");
			setPowerState();
		}
		
		atuoPoweroffCount++;
		if(mPlayinglist->isPlaying()){
			atuoPoweroffCount = 0;
		}
		
		if(gPlayer.mPlayingView != NULL && 
			(gPlayer.mPlayingView->isUsmCon || gPlayer.mPlayingView->isMediaScanning)){
			atuoPoweroffCount = 0;
		}
		
		if(atuoPoweroffCount>=poweroffTime && poweroffTime != -1){
			gMessageQueue.post(gPlayer.mPlayingView,VM_NOTIFY,NM_POWER_OFF,0);
		}
	}

	void PowerManager::resetCount(){
		atuoCount = 0;
		atuoPoweroffCount = 0;
		//log_i("PowerManager::resetCount atuoCount=0");
	}

	void PowerManager::setPowerState(){
		int fd=0;
		char* path = "/sys/power/state";
		char rbuf[6]={};
		const char *const pm_states[5] = {
			"on",
			"standby",
			"mem"
		};
		log_i("PowerManager::setPowerState atuoCount=%d",atuoCount);
		atuoCount = 0;

		if(isSleep)
			isSleep = 0;
		else
			isSleep = 2;
		
		fd = open(path,O_RDWR, 0);
		if(fd==-1)
			return;
		
		if(write(fd,pm_states[isSleep],strlen(pm_states[isSleep])) == -1){
			log_i("setPowerState write fail.");
		}
		if(isSleep == 0){
			gSession.invalidateScreen(NULL);
		}
		gMessageQueue.post(gPlayer.mPlayingView,VM_NOTIFY,POWER_STATUS_CHANGE,isSleep);
		close(fd);
	}

	void PowerManager::setPowerState(int n){
		int fd=0;
		char* path = "/sys/power/state";
		char rbuf[6]={};
		const char *const pm_states[5] = {
			"on",
			"standby",
			"mem"
		};
		
		log_i("direct setPowerState n=%d",n);
		
		atuoCount = 0;

		isSleep = n;

		fd = open(path,O_RDWR, 0);
		if(fd==-1)
			return;
		
		if(write(fd,pm_states[isSleep],strlen(pm_states[isSleep])) == -1){
			log_i("setPowerState write fail.");
		}
		
		close(fd);
	}

	int PowerManager::getAutoSleepTime(){
		int i = -1;
		for(i = 0;i<len;i++){
			if(sleepTime == time[i])
				break;
		}
		return i;
	}
	int PowerManager::getPoweroffTime(){
		int i;
		for(i = 0;i<plen;i++){
			if(poweroffTime == power[i])
				break;
		}
		return i;
	}

	
	PowerManager *gPowerManager = NULL;
};

#include <iostream>
#include <libzippp.h>
#include <vector>
#include <map>
#include <fstream>
#include <boost/filesystem.hpp>
#include <algorithm>
#include "CustomCampaignManager.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x) * 1000)
#endif

namespace fs = boost::filesystem;

CustomCampaignManager ccm;
std::map<std::string, std::string> config;

void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void printMainMenu() {
    clearScreen();
    std::cout<<"        ---CrossCCM---"<<std::endl<<std::endl;
    std::cout<<"1. Install Campaign"<<std::endl;
    std::cout<<"2. Installed Campaigns List"<<std::endl;
    std::cout<<"3. Active Campaigns List"<<std::endl;
    std::cout<<"4. Set Active Campaign"<<std::endl;
    std::cout<<"5. Restore Campaign to Default"<<std::endl;
    std::cout<<"6. Exit"<<std::endl;
}

void init() {
    std::fstream file;

    file.open("xccm_config", std::ios::in | std::ios::out);

    if(!file.good()) {
        std::cout<<"Error loading config file!"<<std::endl;
        exit(1);
    }

    std::string line;
    while(std::getline(file, line)) {
        if(line.back() == '\r' || line.back() == '\n') {
            line.pop_back();
        }
        std::string key = "";
        std::string value = "";
        int pos = line.find("=");
        if(pos == std::string::npos) {
            return;
        }
        for(int i = 0; i < pos; i++) {
            if(line[i]!=' ') {
                std::string c(1, line[i]);
                key += c;
            }
        }
        pos++; //skip '='
        for(int i = pos; i < line.length(); i++) {
            std::string c(1, line[i]);
            value += c;
        }
        config[key] = value;
    }

    file.close();

    if(config.find("config") == config.end() || config.find("sc2root") == config.end()) {
        file.open("xccm_config", std::ios::in | std::ios::out);
        if(ccm.findDefaultRootDir()) {
            file << "sc2root=" + ccm.getSc2Root() << std::endl;
        }
        else {
            std::string path = "";
            while(!fs::is_directory((fs::path)path)) {
                clearScreen();
                std::cout<<"Starcraft 2 root directory not found! \n You have to specify it manually. \n"<<
                "Root directory of Starcraft 2 is directory which contains Starcraft 2 executable file (eg. Starcraft2.exe) \n Input path here >>> ";
                std::getline(std::cin, path);
                if(!fs::is_directory((fs::path)path)) {
                    std::cout<<std::endl<<"Invalid path!"<<std::endl;
                    Sleep(2000);
                }
            }
            #ifdef _WIN32
            path.append("\\");
            #else
            path.append("/");
            #endif
            file << "sc2root=" + path << std::endl;
            ccm.setSc2Root(path);
        }
        file << "config=1" << std::endl;
        file.close();
    }
    else {
        ccm.setSc2Root(config["sc2root"]);
    }

    
}

int main() {
    init();
    while(true) {
        std::string selection;
        printMainMenu();
        std::cin>>selection;
        if(selection == "1") {
            std::string archiveName;
            clearScreen();
            std::cout<<"Campaign's Archive Name \n(or path, if it is in different folder): ";
            std::cin>>archiveName;
            clearScreen();
            if(ccm.unpackCampaign(archiveName) == true && ccm.installUnpackedCampaign() == true) {
                std::cout<<"Installation succesful!"<<std::endl;
                Sleep(1250);
                std::string path = ccm.getLastInstalledCampaignPath();
                if(path != "") {
                    if(ccm.loadMetadataFile(path) == 0) {
                        clearScreen();
                        std::cout<<"Title: "<<ccm.getMetadataKey("title")<<std::endl;
                        std::cout<<"Author: "<<ccm.getMetadataKey("author")<<std::endl;
                        std::cout<<"Version: "<<ccm.getMetadataKey("version")<<std::endl;
                        std::cout<<"Campaign: "<<ccm.getMetadataKey("campaign")<<std::endl<<std::endl;
                    }
                    std::cout<<"Do you want to set this campaign active? [y/n]: "<<std::endl;
                    std::cin>>selection;
                    std::transform(selection.begin(), selection.end(), selection.begin(), ::tolower);
                    if(selection == "y") {
                        if(ccm.setActiveCampaign((fs::path)path) == true) {
                            clearScreen();
                            std::cout<<"Campaign set successfully!"<<std::endl;
                            Sleep(1250);
                        }
                        
                    }
                }
            }
        }
        else if(selection == "2") {
            clearScreen();
            std::map<std::string, std::map<std::string, std::string>> m = ccm.getFullCampaignsList();
            for(const auto& [key, value] : m) {
                std::map<std::string, std::string> meta = value;
                std::cout<<"Title: "<<meta["title"]<<std::endl;
                std::cout<<"Author: "<<meta["author"]<<std::endl;
                std::cout<<"Version: "<<meta["version"]<<std::endl;
                std::cout<<"Campaign: "<<meta["campaign"]<<std::endl;
                std::cout<<"------------------------------"<<std::endl;
            }
            std::cout<<"Press enter to continue..."<<std::endl;
            std::cin.ignore();
            std::cin.get();
        }
        else if(selection == "3") {
            clearScreen();
            std::map<std::string, std::string> m;

            m = ccm.getActiveCampaignData(ccm.WoL);
            std::cout<<"--- Wings of Liberty ---"<<std::endl;
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<"Title: "<<m["title"]<<std::endl;
                std::cout<<"Author: "<<m["author"]<<std::endl;
                std::cout<<"Version: "<<m["version"]<<std::endl;
                std::cout<<"Campaign: "<<m["campaign"]<<std::endl;
            }
            std::cout<<"------------------------"<<std::endl;

            m = ccm.getActiveCampaignData(ccm.HotS);
            std::cout<<"--- Heart of the Swarm ---"<<std::endl;
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<"Title: "<<m["title"]<<std::endl;
                std::cout<<"Author: "<<m["author"]<<std::endl;
                std::cout<<"Version: "<<m["version"]<<std::endl;
                std::cout<<"Campaign: "<<m["campaign"]<<std::endl;
            }
            std::cout<<"------------------------"<<std::endl;

            m = ccm.getActiveCampaignData(ccm.LotV);
            std::cout<<"--- Legacy of the Void ---"<<std::endl;
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<"Title: "<<m["title"]<<std::endl;
                std::cout<<"Author: "<<m["author"]<<std::endl;
                std::cout<<"Version: "<<m["version"]<<std::endl;
                std::cout<<"Campaign: "<<m["campaign"]<<std::endl;
            }
            std::cout<<"------------------------"<<std::endl;

            m = ccm.getActiveCampaignData(ccm.NCO);
            std::cout<<"--- Nova Covert Ops ---"<<std::endl;
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<"Title: "<<m["title"]<<std::endl;
                std::cout<<"Author: "<<m["author"]<<std::endl;
                std::cout<<"Version: "<<m["version"]<<std::endl;
                std::cout<<"Campaign: "<<m["campaign"]<<std::endl;
            }
            std::cout<<"------------------------"<<std::endl;

            std::cout<<"Press enter to continue..."<<std::endl;
            std::cin.ignore();
            std::cin.get();
        }
        else if(selection == "4") {
            clearScreen();
            std::map<std::string, fs::path> m = ccm.getCampaignsList();
            int i = 1;
            for(const auto& [key, value] : m) {
                std::cout<<i<<". "<<key<<std::endl;
                i++;
            }
            std::string j;
            std::cout<<"Which campaing you want to set active? : ";
            std::cin>>j;
            if(std::stoi(j) < i && std::stoi(j) > 0) {
                i = 1;
                for(const auto& [key, value] : m) {
                    if(i == stoi(j)) {
                        if(ccm.setActiveCampaign((fs::path)value) == true) {
                            clearScreen();
                            std::cout<<"Campaign set successfully!"<<std::endl;
                            Sleep(1250);
                        }
                        else {
                            clearScreen();
                            std::cout<<"Error occured, campaign is not set!"<<std::endl;
                            Sleep(1250);
                        }
                    }
                    i++;
                }
            }
            else {
                clearScreen();
                std::cout<<"Invalid Input!"<<std::endl;
                Sleep(1250);
            }
        }
        else if(selection == "5") {
            clearScreen();
            std::map<std::string, std::string> m;
            m = ccm.getActiveCampaignData(ccm.WoL);
            std::cout<<"1. [Wings of Liberty]: ";
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<ccm.getMetadataKey("title")<<" [Version "<<ccm.getMetadataKey("version")<<"]"<<std::endl;
            }

            m = ccm.getActiveCampaignData(ccm.HotS);
            std::cout<<"2. [Heart of the Swarm]: ";
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<ccm.getMetadataKey("title")<<" [Version "<<ccm.getMetadataKey("version")<<"]"<<std::endl;
            }

            m = ccm.getActiveCampaignData(ccm.LotV);
            std::cout<<"3. [Legacy of the Void]: ";
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<ccm.getMetadataKey("title")<<" [Version "<<ccm.getMetadataKey("version")<<"]"<<std::endl;
            }

            m = ccm.getActiveCampaignData(ccm.NCO);
            std::cout<<"4. [Nova Covert Ops]: ";
            if(m.empty()) {
                std::cout<<"Default Campaign"<<std::endl;
            }
            else {
                std::cout<<ccm.getMetadataKey("title")<<" [Version "<<ccm.getMetadataKey("version")<<"]"<<std::endl;
            }

            std::cout<<"Which campaign you want to restore to default? : ";
            std::cin>>selection;

            if(selection == "1") {
                ccm.restartExpansion(ccm.WoL);
            }
            else if(selection == "2") {
                ccm.restartExpansion(ccm.HotS);
            }
            else if(selection == "3") {
                ccm.restartExpansion(ccm.LotV);
            }
            else if(selection == "4") {
                ccm.restartExpansion(ccm.NCO);
            }

            clearScreen();
            std::cout<<"Default campaign restored!"<<std::endl;
            Sleep(1250);
        }
        else if(selection == "6") {
            clearScreen();
            exit(0);
        }
        else {
            clearScreen();
            std::cout<<"Invalid Choice!"<<std::endl;
            Sleep(2000);
        }
    }
    return 0;
}
import os, sys, argparse, itertools, json
from collections import OrderedDict as OD

c0_inputFile = 'Settings/settings_background.json'
c0_settings = json.loads(open(c0_inputFile).read(),object_pairs_hook=OD)
c0_settings_list = [c0_settings[key] for key in c0_settings]
c0_settings_products = list(itertools.product(*c0_settings_list))

c1_inputFile = 'Settings/settings_c1.json'
c1_settings = json.loads(open(c1_inputFile).read(),object_pairs_hook=OD)
c1_settings_list = [c1_settings[key] for key in c1_settings]
c1_settings_products = list(itertools.product(*c1_settings_list))

c2_inputFile = 'Settings/settings_c2.json'
c2_settings = json.loads(open(c2_inputFile).read(),object_pairs_hook=OD)
c2_settings_list = [c2_settings[key] for key in c2_settings]
c2_settings_products = list(itertools.product(*c2_settings_list))

def GetInFilename(channel,mass,n,nf,ef):
    return '/pnfs/uboone/scratch/users/sporzio/HeavySterileNeutrinos/sterileEvents_channel%s_mass%s_n%s_nf%s_ef%s/Log/reco/files.list' %(channel,mass,n,nf,ef)

def GetOutFilename(channel,mass,n,nf,ef,dCut,po,eva,at):
    if channel!="0":
        return 'Output/sterileEvents_channel%s_mass%s_n%s_nf%s_ef%s_dCut%s_po-%s_eva-%s_at-%s.root' %(channel,mass,n,nf,ef,dCut,po,eva,at)
    else:
        return 'Output/background_channel%s_mass%s_n%s_nf%s_ef%s_dCut%s_po-%s_eva-%s_at-%s.root' %(channel,mass,n,nf,ef,dCut,po,eva,at)

def GetCommand(inputFile,outputFile,dCut,po,eva,at):
    return './GeoCuts_Radius %s %s %s %s %s %s' %(inputFile,outputFile,dCut,po,eva,at)


commandFile = open('commandFile.sh','w')
# for setting in c1_settings_products:
#     inputFile = GetInFilename(setting[0],setting[1],setting[2],setting[3],setting[4])
#     outputFile = GetOutFilename(setting[0],setting[1],setting[2],setting[3],setting[4],setting[5],setting[6],setting[7],setting[8])
#     command = GetCommand(inputFile,outputFile,setting[5],setting[6],setting[7],setting[8])
#     commandFile.write(command+'\n')
# for setting in c2_settings_products:
#     inputFile = GetInFilename(setting[0],setting[1],setting[2],setting[3],setting[4])
#     outputFile = GetOutFilename(setting[0],setting[1],setting[2],setting[3],setting[4],setting[5],setting[6],setting[7],setting[8])
#     command = GetCommand(inputFile,outputFile,setting[5],setting[6],setting[7],setting[8])
#     commandFile.write(command+'\n')
for setting in c0_settings_products:
    inputFile = '/uboone/app/users/sporzio/SterileNeutrinos/RecoCuts/Lists/prod_reco2_bnb_v6_ddr_mcc8_test4_n1000.list'
    outputFile = GetOutFilename(setting[0],setting[1],setting[2],setting[3],setting[4],setting[5],setting[6],setting[7],setting[8])
    command = GetCommand(inputFile,outputFile,setting[5],setting[6],setting[7],setting[8])
    commandFile.write(command+'\n')
commandFile.close()

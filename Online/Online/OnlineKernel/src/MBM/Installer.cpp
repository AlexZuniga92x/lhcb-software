#define MBM_IMPLEMENTATION
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "bm_struct.h"
#include "Installer.h"

static void help()  {
  ::lib_rtl_printf("Syntax: mbm_install/mbm_deinstall [<-opt>]\n");
  ::lib_rtl_printf("Function: Buffer Manager Installation\n");
  ::lib_rtl_printf("Options:\n");
  ::lib_rtl_printf("    -s=<size> [10]      Buffer size (kbytes)\n");
  ::lib_rtl_printf("    -e=<max>  [32]      Maximum number of events\n");
  ::lib_rtl_printf("    -u=<max>  [5]       Maximum number of users\n");
  ::lib_rtl_printf("    -i=<id>   [ ]       Buffer Identifier \n");
  ::lib_rtl_printf("    -f        [ ]       force deinstall\n");
  ::lib_rtl_printf("    -m        [ ]       Start monitor after installer\n");
  ::lib_rtl_printf("    -c        [ ]       Do not keep process alive; continue execution\n");
}

MBM::Installer::Installer(int argc, char **argv)  {
  p_continue = 0;
  p_moni   = 0;
  p_emax   = 32;
  p_umax   = 5;
  p_size   = 10;
  p_force  = 0;
  m_bm = new BMDESCRIPT;
  ::memset(m_bm,0,sizeof(BMDESCRIPT));
  getOptions(argc, argv);
}

int MBM::Installer::optparse (const char* c)  {
  int iret;
  switch (*c | 0x20)    {
  case 's':            /*      size of buffer        */   
    iret = sscanf(c+1,"=%d",&p_size);
    if( iret != 1 )       {
      ::lib_rtl_printf("Error reading buffer size parameter\n");
      throw std::runtime_error("Error reading buffer size parameter");
    }
    p_size = ((p_size+1)>>1)<<1;
    break;
  case 'e':            /*      maximum events        */   
    iret = sscanf(c+1,"=%d",&p_emax);
    if( iret != 1 )       {
      ::lib_rtl_printf("Error reading maximum events parameter\n");
      throw std::runtime_error("Error reading  maximum events parameter");
    }
    break;
  case 'u':            /*      maximum users        */   
    iret = sscanf(c+1,"=%d",&p_umax);
    if( iret != 1 )  {
      ::lib_rtl_printf("Error reading maximum users parameter\n");
      throw std::runtime_error("Error reading  maximum users parameter");
    }
    if( p_umax > 128 )    {
      ::lib_rtl_printf("Maximum users exeeded maximum (128)\n");
      throw std::runtime_error("Maximum users exeeded maximum (128)");
    }
    break;
  case 'i':            /*      maximum users        */   
    iret = sscanf(c+1,"=%s",buff_id);
    if( iret != 1 )       {
      ::lib_rtl_printf("Error reading Buffer identifier parameter\n");
      throw std::runtime_error("Error reading Buffer identifier parameter");
    }
    ::strcpy(m_bm->bm_name,buff_id);
    bm_id = buff_id;
    break;
  case 'f':
    p_force = 1;
    break;
  case 'm':
    p_moni = 1;
    break;
  case 'c':
    p_continue = 1;
    break;
  case '?':
  case 'h':
    help();
    throw std::runtime_error("Help only requested");
  default:
    help();
    throw std::runtime_error("Bad parameter supplied!");
  }
  return MBM_NORMAL;
}

int MBM::Installer::install()  {
  int len, icode = deinstall();
  if(icode != MBM_NORMAL) return icode;
  ::lib_rtl_printf("++bm_init++ Commencing BM installation \n");
  int status = ::lib_rtl_create_section(ctrl_mod,sizeof(CONTROL),&m_bm->ctrl_add);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_printf("Cannot create section %s. Exiting....",ctrl_mod);
    return status;
  }
  m_bm->ctrl = (CONTROL*)m_bm->ctrl_add->address;
  ::memset(m_bm->ctrl,0,sizeof(CONTROL));
  ::lib_rtl_printf("Control: %p  %08X             [%d Bytes]\n",(void*)m_bm->ctrl,
           ((char*)m_bm->ctrl)-((char*)m_bm->ctrl), sizeof(CONTROL));

  len = sizeof(USERDesc)+sizeof(USER)*(p_umax-1);
  status = ::lib_rtl_create_section(user_mod, len ,&m_bm->user_add);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_delete_section(m_bm->ctrl_add);
    ::lib_rtl_printf("Cannot create section %s. Exiting....",user_mod);
    return status;
  }
  m_bm->usDesc = (USERDesc*)m_bm->user_add->address;
  m_bm->user   = m_bm->usDesc->users;
  ::memset(m_bm->usDesc,0,sizeof(USERDesc));
  ::memset(m_bm->user,0,sizeof(USER)*p_umax);
  ::lib_rtl_printf("User:    %p  %08X  %p   [%d Bytes]\n",(void*)m_bm->user,
           ((char*)m_bm->user)-((char*)m_bm->ctrl),(void*)m_bm->usDesc, len);

  len = sizeof(EVENTDesc)+sizeof(EVENT)*(p_emax-1);
  status = ::lib_rtl_create_section(event_mod, len ,&m_bm->event_add);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_delete_section(m_bm->ctrl_add);
    ::lib_rtl_delete_section(m_bm->user_add);
    ::lib_rtl_printf("Cannot create section %s. Exiting....",event_mod);
    return status;
  }
  m_bm->evDesc = (EVENTDesc*)m_bm->event_add->address;
  m_bm->event = m_bm->evDesc->events;
  ::memset(m_bm->evDesc,0,len);
  ::lib_rtl_printf("Event:   %p  %08X  %p   [%d Bytes]\n",(void*)m_bm->event,
           ((char*)m_bm->event)-((char*)m_bm->ctrl),(void*)m_bm->evDesc, len);

  len = (p_size<<Bits_p_kByte)>>3;
  status = ::lib_rtl_create_section(bitmap_mod,len,&m_bm->bitm_add);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_delete_section(m_bm->ctrl_add);
    ::lib_rtl_delete_section(m_bm->user_add);
    ::lib_rtl_delete_section(m_bm->event_add);
    ::lib_rtl_printf("Cannot create section %s. Exiting....",bitmap_mod);
    return status;
  }
  m_bm->bitmap = (char*)m_bm->bitm_add->address;
  ::memset(m_bm->bitmap,0,len);
  ::lib_rtl_printf("Bitmap:  %p  %08X             [%d Bytes]\n",(void*)m_bm->bitmap,
           ((char*)m_bm->bitmap)-((char*)m_bm->ctrl), len);

  len = p_size<<10;
  status = ::lib_rtl_create_section(buff_mod,len,&m_bm->buff_add);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_delete_section(m_bm->ctrl_add);
    ::lib_rtl_delete_section(m_bm->user_add);
    ::lib_rtl_delete_section(m_bm->event_add);
    ::lib_rtl_delete_section(m_bm->buff_add);
    ::lib_rtl_printf("Cannot create section %s. Exiting....",buff_mod);
    return status;
  }
  m_bm->buffer_add = (char*)m_bm->buff_add->address;
  ::memset(m_bm->buffer_add,0xDD,len);
  ::lib_rtl_printf("Buffer:  %p  %08X             [%d Bytes]\n",(void*)m_bm->buffer_add,
           ((char*)m_bm->buffer_add)-((char*)m_bm->ctrl), len);

  CONTROL* ctrl  = m_bm->ctrl;
  USER*    user  = m_bm->user;
  EVENT*   event = m_bm->event;
  ctrl->p_umax       = p_umax;
  ctrl->p_emax       = p_emax;
  ctrl->buff_size    = p_size<<10; /* in bytes*/
  ctrl->tot_produced = 0;
  ctrl->tot_actual   = 0;
  ctrl->tot_seen     = 0;
  ctrl->i_events     = 0;
  ctrl->i_space      = p_size*Bits_p_kByte; /*in Bits*/
  ctrl->last_bit     = 0;
  ctrl->last_alloc   = 0;
  ctrl->bm_size      = p_size*Bits_p_kByte; /*in bits*/
  ctrl->spare1       = 0;
  for (int i=0;i<p_umax;i++)  {
    user[i].block_id = BID_USER;
    user[i].uid      = i;
  }
  for (int j=0;j<p_emax;j++)  {
    event[j].block_id = BID_EVENT;
    event[j].eid      = j;
  }
  status = mbm_map_global_buffer_info(&bm_all);
  if(!::lib_rtl_is_success(status))   {   
    ::lib_rtl_delete_section(m_bm->ctrl_add);
    ::lib_rtl_delete_section(m_bm->user_add);
    ::lib_rtl_delete_section(m_bm->event_add);
    ::lib_rtl_delete_section(m_bm->buff_add);
    ::lib_rtl_printf("Cannot map global buffer information....\n");
    return status;
  }
  BUFFERS* buffs = (BUFFERS*)bm_all->address;
  for(int i=0; i<buffs->p_bmax; ++i)  {
    if ( 0 == buffs->buffers[i].used )  {
      buffs->buffers[i].used = 1;
      ::strcpy(buffs->buffers[i].name, m_bm->bm_name);
      buffs->nbuffer++;
      ::lib_rtl_printf("++bm_init++ BM registration successful:%d buffers now \n",buffs->nbuffer);
      break;
    }
  }
  ::lib_rtl_printf("++bm_init++ BM installation [%s] successfully. \n",m_bm->bm_name);
  return MBM_NORMAL;
}

int MBM::Installer::deinstall()  {
  ::lib_rtl_gbl_t handle;
  ::lib_rtl_printf("++bm_init++ Commencing BM deinstallation \n");
  int status = ::lib_rtl_map_section(ctrl_mod, sizeof(CONTROL), &handle);
  if( !::lib_rtl_is_success(status)) 
    return 1;
  if (p_force != 1)  {
    m_bm->ctrl = (CONTROL*)handle->address;
    if( m_bm->ctrl->i_users > 0 )    {
      ::lib_rtl_printf("++bm_init++ Unable to de-install BM (%d users still active)\n", m_bm->ctrl->i_users);
      return MBM_ERROR;
    }
  }
  status = ::lib_rtl_delete_section(handle);
  if (!::lib_rtl_is_success(status))  {
    ::lib_rtl_printf("problem deleting section %s status %d\n",ctrl_mod,status);
  }
  status = ::lib_rtl_map_section(user_mod, sizeof(USER), &handle);
  status = ::lib_rtl_delete_section(handle);
  if (!::lib_rtl_is_success(status))  {
    ::lib_rtl_printf("problem deleting section %s status %d\n",user_mod,status);
  }
  status = ::lib_rtl_map_section(buff_mod, 1, &handle);
  status = ::lib_rtl_delete_section(handle);
  if (!::lib_rtl_is_success(status))  {
    ::lib_rtl_printf("problem deleting section %s status %d\n",buff_mod,status);
  }
  status = ::lib_rtl_map_section(event_mod, 1, &handle);
  status = ::lib_rtl_delete_section(handle);
  if (!::lib_rtl_is_success(status))  {
    ::lib_rtl_printf("problem deleting section %s status %d\n",event_mod,status);
  }
  status = ::lib_rtl_map_section(bitmap_mod, 1, &handle);
  status = ::lib_rtl_delete_section(handle);
  if (!::lib_rtl_is_success(status))  {
    ::lib_rtl_printf("problem deleting section %s status %d\n",bitmap_mod,status);
  }
  status = mbm_map_global_buffer_info(&bm_all);
  if(!::lib_rtl_is_success(status))
    return 1;
  BUFFERS* buffs = (BUFFERS*)bm_all->address;
  for(int i=0; i<buffs->p_bmax; ++i)  {
    if ( ::strcmp(buffs->buffers[i].name,bm_id)==0 )  {
      buffs->buffers[i].used = 0;
      buffs->nbuffer--;
      ::memset(buffs->buffers[i].name,0,sizeof(buffs->buffers[i].name));
      break;
    }
  }
  mbm_unmap_global_buffer_info(bm_all);
  ::lib_rtl_printf("++bm_init++ Old BM [%s] de-installed successfully\n",m_bm->bm_name);
  return MBM_NORMAL;
}

int mbm_install(int argc , char** argv) {
  try  {
    MBM::Installer inst(argc, argv);
    int sc = inst.install();
    if ( sc == MBM_NORMAL )  {
      if ( inst.startMonitor() )  {
        lib_rtl_sleep(1000);
        mbm_mon(0, argv); 
      }
      else if ( !inst.continueInstallation() ) {
        for(;;) ::lib_rtl_sleep(10000);
      }
    }
    ::lib_rtl_printf("All done.\n");
    return sc;
  }
  catch (std::exception& e)  {
    lib_rtl_printf("MBM initialization failed: %s\n",e.what());
  }
  return MBM_ERROR;
}

int mbm_deinstall(int argc , char** argv) {
  MBM::Installer inst(argc, argv);
  return inst.deinstall();
}

if ( !lhcb.widgets ) {

  lhcb.widgets = new Object();

  /// Check initialization
  lhcb.widgets.check = function() {};
  
  /// Set document location to LHCb DAQ status page
  lhcb.widgets.goto_lhcb_page = function(name) {
    var item = function() {    document.location = this.url; };
    item.url =  lhcb.constants.lhcb_display_url(name);
    return item;
  };
  /// Set document location to LHCb DAQ status page
  lhcb.widgets.goto_lhcb_daq_page = function() {
    document.location =  lhcb.constants.lhcb_display_url("status&system=LHCb");
  };
  /// Set document location to LHC operations page
  lhcb.widgets.goto_lhc_operations_page = function() {
    document.location =  lhcb.constants.operations_url("LHC3");
  };

  lhcb.widgets.green_smiley  = function() { 
    return '<IMG src="'+lhcb.constants.mkStaticImage('smileys/smiley4162.gif').src+'" height="25"></IMG>';   
  };
  lhcb.widgets.yellow_smiley = function() {
    return '<IMG src="'+lhcb.constants.mkStaticImage('smileys/smiley1807.gif').src+'" height="25"></IMG>';   
  };
  lhcb.widgets.red_smiley    = function() {
    return '<IMG src="'+lhcb.constants.mkStaticImage('smileys/free-mad-smileys-120.gif').src+'" height="25"></IMG>';
  };
  lhcb.widgets.blue_smiley   = function() { 
    return '<IMG src="'+lhcb.constants.mkStaticImage('smileys/smiley153.gif').src+'" height="15"></IMG>';   
  };

  lhcb.widgets.button = function(value,onclick) {
    var but, cell = document.createElement('td');
    if ( _isInternetExplorer() ) {
      cell.appendChild(but=document.createElement('button'));
    }
    else {
      cell.appendChild(but=document.createElement('input'));
      but.type = 'button';
    }
    but.value = value;
    but.onclick = onclick;
    return [cell,but];
  };

  lhcb.widgets.subscribeItem = function(provider,item) {
    if ( item ) {
      provider.subscribe(item.name,item);
    }
  };

  lhcb.widgets.mkFSMitem1 = function(item,label) {
    var tr = document.createElement('tr');
    if ( label )
      tr.appendChild(Cell(label,1,'FSMLabel'));
    else
      tr.appendChild(item.label);
    tr.appendChild(item);
    tr.appendChild(item.lock);
    item.lock.className = null;
    return tr;
  };


  /** Build table with summary of the LHC
   *   
   * @return Reference to HTML table containing all data items of this widget
   *
   * @author  M.Frank
   * @version 1.0
   */
  lhcb.widgets.LHC_header = function() {
    var tb, td, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
  
    tab.className = tb.className = 'MonitorSubHeader';
  
    tb.appendChild(tr=document.createElement('tr'));
  
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = 'LHC&nbsp;Fill';
  
    tab.lhcFillNumber = StyledItem('lbWeb.LHCCOM/LHC.LHC.RunControl.FillNumber',null,null);
    tr.appendChild(tab.lhcFillNumber);
  
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = 'is&nbsp;in&nbsp;state:';
  
    tab.lhcBeamMode = StyledItem('lbWeb.LHCCOM/LHC.LHC.RunControl.BeamMode',null,null);
    tr.appendChild(tab.lhcBeamMode);
  
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = '';
  
    tab.appendChild(tb);

    /** Subscribe all required data item to receive data from data provider object
     *
     *  @param      provider  Data provider object
     *
     *  @return On success reference to self, null otherwise
     */
    tab.subscribe = function(provider) {
      this.lhcBeamMode.subscribe(provider);
      this.lhcFillNumber.subscribe(provider);
      return this;
    };
    return tab;
  };
 
  /** Build table with LHC summary information
   *   
   * @return Reference to HTML table containing all data items of this widget
   *
   * @author  M.Frank
   * @version 1.0
   */
  lhcb.widgets.LHC_summary = function() {
    var c, tr, tab = document.createElement('table');
    var tb = document.createElement('tbody');

    tab.className = tb.className  = 'MonitorPage';
    //tab.height    = '120px';

    tab.energy       = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Energy',null,"%7.0f GeV");
    //tab.intensity1   = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Intensity.Beam1.totalIntensity',null,"%9.2e");
    //tab.intensity2   = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Intensity.Beam2.totalIntensity',null,"%9.2e");
    //tab.lifetime1    = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Intensity.Beam1.primitiveLifetime',null,"%7.2f");
    //tab.lifetime2    = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Intensity.Beam2.primitiveLifetime',null,"%7.2f");
    tab.intensity1    = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.IntensityPerBunch.Beam1.averageBeamIntensity',null,'%9.2e');
    tab.intensity2    = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.IntensityPerBunch.Beam2.averageBeamIntensity',null,'%9.2e');
    tab.lifetime1     = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.IntensityPerBunch.Beam1.bestLifetime',null,'%7.2f h');
    tab.lifetime2     = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.IntensityPerBunch.Beam2.bestLifetime',null,'%7.2f h');

    tab.avgLumi      = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Beam.Luminosity.LuminosityAverage',null,"%9.2e");
    tab.LHCbDump     = StyledItem('lbWeb.LHCCOM/LHC.LHC.Handshake.LHC_BEAMDUMP',null,null);
    tab.LHCbAdjust   = StyledItem('lbWeb.LHCCOM/LHC.LHC.Handshake.LHC_ADJUST',null,null);
    tab.LHCbInject   = StyledItem('lbWeb.LHCCOM/LHC.LHC.Handshake.LHC_INJECTION',null,null);

    tab.energy.conversion = function(value) {  return value*120.0/1000.0; };

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(Cell('LHC status summary',4,'MonitorDataHeader'));

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(c=Cell('Energy:',null,'MonitorDataHeader'));
    c.style.width = '30%';
    tr.appendChild(tab.energy);
    tr.appendChild(Cell('Avg.Luminosity:',null,'MonitorDataHeader'));
    tr.appendChild(tab.avgLumi);

    tr = document.createElement('tr');
    tb.appendChild(tr);

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(Cell('Intensity [e]:',null,'MonitorDataHeader'));
    tr.appendChild(tab.intensity1);
    tr.appendChild(Cell('&larr;1 Beam 2&rarr;',null,'Text-Center'));
    tr.appendChild(tab.intensity2);

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(Cell('Lifetime [h]:',null,'MonitorDataHeader'));
    tr.appendChild(tab.lifetime1);
    tr.appendChild(Cell('&larr;1 Beam 2&rarr;',null,'Text-Center'));
    tr.appendChild(tab.lifetime2);

    tr = document.createElement('tr');
    tb.appendChild(tr);
    c = Cell('Handshakes:',null,'MonitorDataHeader');
    c.rowSpan = 2;
    tr.appendChild(c);
    tr.appendChild(Cell('Dump',null,'MonitorDataHeader'));
    tr.appendChild(Cell('Adjust',null,'MonitorDataHeader'));
    tr.appendChild(Cell('Injection',null,'MonitorDataHeader'));

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(tab.LHCbDump);
    tr.appendChild(tab.LHCbAdjust);
    tr.appendChild(tab.LHCbInject);
    tab.LHCbDump.width   = '70px';
    tab.LHCbAdjust.width = '70px';
    tab.LHCbInject.width = '70px';

    /*
    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(tab.LumiFill);
    */


    tab.appendChild(tb);

    /** Subscribe all required data item to receive data from data provider object
     *
     *  @param      provider  Data provider object
     *
     *  @return On success reference to self, null otherwise
     */
    tab.subscribe = function(provider) {
      this.energy.subscribe(provider);
      this.intensity1.subscribe(provider);
      this.intensity2.subscribe(provider);
      this.lifetime1.subscribe(provider);
      this.lifetime2.subscribe(provider);
      this.avgLumi.subscribe(provider);
      this.LHCbDump.subscribe(provider);
      this.LHCbAdjust.subscribe(provider);
      this.LHCbInject.subscribe(provider);
      return this;
    };
    return tab;
  };

  /**
     Build table like this:

     +-------- LHC operator comments ----------
     |LHC in limbo, awaiting decision ...      
     |
     +-----------------------------------------
  */
  lhcb.widgets.LHC_operator_comments = function() {
    var tb, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
    tab.className = tb.className = 'MonitorPage';
    
    tab.comments   = StyledItem('lbWeb.LHCCOM/LHC.LHC.RunControl.Page1Comment',null,null);
    tab.comments.style.fontWeight = 'normal';
    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(Cell('LHC operator comments:',null,'MonitorDataHeader'));
    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(tab.comments);
    tab.comments.conversion = function(data) { return data.replace(/\n/g,'<BR>');  };
    tab.comments.height = '60px';

    tab.appendChild(tb);

    /** Subscribe all required data item to receive data from data provider object
     *
     *  @param      provider  Data provider object
     *
     *  @return On success reference to self, null otherwise
     */
    tab.subscribe = function(provider) {
      this.comments.subscribe(provider);
      return this;
    };
    return tab;
  };

  /**
     Build table like this:

     +---- Shift crew : None --------------------------------------------------------
     |
     |
     +---- Comment updated on 18-SEP-2000 16:28 -------------------------------------
  */
  lhcb.widgets.LHCb_shift_comments = function() {
    var tb, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
    tab.className = tb.className = 'MonitorPage';

    tab.comments   = StyledItem('lbWeb.shiftComments',null,null);
    tab.comments.conversion = function(data) { return data.replace(/\n/g,'<BR>');    };
    tab.comments.style.fontWeight = 'normal';

    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(Cell('Shift comments:',null,'MonitorDataHeader'));
    tr = document.createElement('tr');
    tb.appendChild(tr);
    tr.appendChild(tab.comments);
    tab.comments.height = '100px';
    tab.appendChild(tb);

    /** Subscribe all required data item to receive data from data provider object
     *
     *  @param      provider  Data provider object
     *
     *  @return On success reference to self, null otherwise
     */
    tab.subscribe = function(provider) {
      this.comments.subscribe(provider);
      return this;
    };
    return tab;
  };

  lhcb.widgets.LHCb_PlanOfDay = function(options) {
    var tb, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
    tab.className = tb.className = 'MonitorPage';

    tab.comments   = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Internal.Plan',null,null);
    tab.comments.conversion = function(data) { return data.replace(/\n/g,'<BR>');    };
    tab.comments.style.fontWeight = 'normal';

    if ( options.style) {
      tr = document.createElement('tr');
      tb.appendChild(tr);
      tr.appendChild(Cell('Plan of the day:',null,options.style));
    }
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(tab.comments);
    tab.comments.height = '100px';
    tab.appendChild(tb);

    /** Subscribe all required data item to receive data from data provider object
     *
     *  @param      provider  Data provider object
     *
     *  @return On success reference to self, null otherwise
     */
    tab.subscribe = function(provider) {
      this.comments.subscribe(provider);
      return this;
    };
    return tab;
  };

  /** 
   *   
   * @return Reference to HTML table containing all data items of this widget
   *
   * @author  M.Frank
   * @version 1.0
   */
  lhcb.widgets.Det_Run_header = function(sys,logger) {
    var td, tr, tab = document.createElement('table');
    var tb  = document.createElement('tbody');

    tab.style.height = tb.style.height = '100%';
    tab.className = tb.className = 'MonitorSubHeader';

    tab.runNumber = StyledItem('lbWeb.'+sys+'_RunInfo.general.runNumber',null,null);
    tab.runState = FSMItem('lbWeb.'+sys,logger,true);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = 'Run';    
    tr.appendChild(tab.runNumber);
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = ' is ';
    tr.appendChild(tab.runState);
    tr.appendChild(tab.runState.lock);
    tr.appendChild(td=document.createElement('td'));
    td.innerHTML = '';

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribe(this.runNumber.name,this.runNumber);
      provider.subscribe(this.runState.name,this.runState);
      return this;
    };
    return tab;
  };

  /**
     Build table like this:
     +-----------------------------------------
     | RUN_196 , Started  2-NOV-2000 08:02
     |           Last 4 mn     Run    Fill
     | Events            0       0       0
     | LCAL              0       0       0
     | QQbar             0       0       0
     | L1 rate Hz    0.000   0.000 Ineff.(%)
     | L2 rate Hz    0.000   0.000 Op   0.00
     | L3 rate Hz    0.000   0.000 DAQ  0.00
     | Deadtime %     0.00    0.00 Dt   0.00
     | Downtime %     0.00    0.00 Tot  0.00
     | Magnet          3.1    12.1     9.9
     | BKG    TPC    -0.80  SAMBA     0.00
     +-----------------------------------------
  */
  lhcb.widgets.Det_Run_Summary = function(opts) {
    var sys = opts.system;
    var logger = opts.logger;
    var tb, td, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');

    tab.className  = tb.className = 'MonitorPage';
    tb.cellSpacing = 0;
    tb.cellPadding = 0;

    tab.runType        = StyledItem('lbWeb.'+sys+'_RunInfo.general.runType',     null,null);
    tab.runStart       = StyledItem('lbWeb.'+sys+'_RunInfo.general.runStartTime',null,'Started at: %s');
    tab.nTriggers      = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.nTriggers',       null,null);
    tab.hltNTriggers   = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.hltNTriggers',null,null);
    tab.deadTime       = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.deadTime',        null,'Now:%8.2f %%');
    tab.deadTimeRun    = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.runDeadTime',     null,'Run:%8.2f %%');
    tab.trgConfig      = StyledItem('lbWeb.'+sys+'_RunInfo.Trigger.TCKLabel',    null,null);
    tab.magnetField    = StyledItem('lbWeb.lbHyst.B',                            null,'%7.4f');
    tab.magnetPolarity = StyledItem('lbWeb.lbHyst.Polarity',                     null, null);
    tab.hvState        = StyledItem('lbWeb.LHCb_LHC_HV.FSM.state','MonitorDataHeader',null);
    tab.hvState.conversion = function(data) {  return 'LV & HV:'+(data.split('/')[1]); };
    tab.veloPosition   = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Specific.VELO.Position','MonitorDataHeader','VELO %s');
    tab.figureOfMerit1 = StyledItem('lbWeb.BCM_DP_S0.RS2_REL',                   null, 'S0:%7.3f');
    tab.figureOfMerit2 = StyledItem('lbWeb.BCM_DP_S0.RS32_REL',                  null, '%7.3f');
    tab.figureOfMerit3 = StyledItem('lbWeb.BCM_DP_S1.RS2_REL',                   null, 'S1:%7.3f');
    tab.figureOfMerit4 = StyledItem('lbWeb.BCM_DP_S1.RS32_REL',                  null, '%7.3f');
    tab.bcmBeamPermit1 = StyledItem('lbWeb.BCM_Interface.BeamPermit.getStatus',  null, null);
    tab.bcmBeamPermit2 = StyledItem('lbWeb.BCM_Interface.InjPermit1.getStatus',  null, null);
    tab.bcmBeamPermit3 = StyledItem('lbWeb.BCM_Interface.InjPermit2.getStatus',  null, null);
    tab.interActionRate= StyledItem('lbWeb.LHCCOM/LHC.LHCb.Internal.TriggerRates.TrgRateLumi_GP', null, '%7.0f Hz');

    tab.l0Rate         = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.triggerRate',     'Text-Right','%8.0f');
    tab.l0RateRun      = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.runTriggerRate',  'Text-Right','%8.0f');
    tab.hltRate        = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.hltRate',     'Text-Right','%8.0f');
    tab.hltRateRun     = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.runHltRate',  'Text-Right','%8.0f');
    tab.physicsRateEx  = StyledItem('lbWeb.LHCB_STATS.PhysEx',  'Text-Right',   '%7.0f');
    tab.physicsRateIn  = StyledItem('lbWeb.LHCB_STATS.PhysIn',  'Text-Right',   '%7.0f');
    tab.LumiRateEx     = StyledItem('lbWeb.LHCB_STATS.LumiEx',  'Text-Right',   '%7.0f');
    tab.LumiRateIn     = StyledItem('lbWeb.LHCB_STATS.LumiIn',  'Text-Right',   '%7.0f');
    tab.LumiRateEx     = StyledItem('lbWeb.LHCB_STATS.LumiEx',  'Text-Right',   '%7.0f');
    tab.LumiRateIn     = StyledItem('lbWeb.LHCB_STATS.LumiIn',  'Text-Right',   '%7.0f');
    tab.minbRateEx     = StyledItem('lbWeb.LHCB_STATS.MBiasEx', 'Text-Right',   '%7.0f');
    tab.minbRateIn     = StyledItem('lbWeb.LHCB_STATS.MBiasIn', 'Text-Right',   '%7.0f');
    tab.instantLumi    = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Internal.Luminosity.LumiInst_GP', null, '%7.2f &mu;b<sup>-1</sup>/s');

    // Run status
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(tab.runType);
    tab.runStart.colSpan = 5;
    tr.appendChild(tab.runStart);
    tab.runStart.style.textAlign = 'left';

    // Magnet status
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Magnet [&nbsp;T&nbsp;]:',2,'MonitorDataHeader'));
    tr.appendChild(tab.magnetField);
    tr.appendChild(Cell('Polarity:',2,'MonitorDataHeader'));
    tr.appendChild(tab.magnetPolarity);
    tab.magnetPolarity.conversion = function(data) {
      if ( data>0 ) return '+&nbsp;(Down)';
      return '-&nbsp;(Up)';
    };
    tr.appendChild(tab.hvState);

    // Background status
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('BCM BKG [&permil;]:',2,'MonitorDataHeader'));
    tr.appendChild(tab.figureOfMerit1);
    tr.appendChild(td=tab.figureOfMerit2);
    td.colSpan = 2;
    tr.appendChild(tab.figureOfMerit3);
    tr.appendChild(tab.figureOfMerit4);

    // Beam permits
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Beam Permits:',2,'MonitorDataHeader'));
    tr.appendChild(td=tab.bcmBeamPermit1);
    td.colSpan = 2;
    tr.appendChild(tab.bcmBeamPermit2);
    tr.appendChild(tab.bcmBeamPermit3);
    tr.appendChild(tab.veloPosition);    // Velo position

    // Luminosity and interaction information
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Interaction rate:',2,'MonitorDataHeader'));
    tr.appendChild(tab.interActionRate);
    tr.appendChild(Cell('Inst.Lumi:',2,'MonitorDataHeader'));
    tr.appendChild(tab.instantLumi);
    tab.instantLumi.colSpan = 2;

    // Event processing information
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('L0 Events:',2,'MonitorDataHeader'));
    tr.appendChild(tab.nTriggers);
    tr.appendChild(Cell('Dead time:',2,'MonitorDataHeader'));
    //tr.appendChild(tab.hltNTriggers);
    tr.appendChild(tab.deadTime);
    tr.appendChild(tab.deadTimeRun);
    //tab.hltNTriggers.colSpan = 2;

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Rates:',1,'MonitorDataHeader'));
    tr.appendChild(Cell('L0&nbsp;&nbsp;[Hz]', 1,'MonitorDataHeader'));
    tr.appendChild(Cell('HLT&nbsp;[Hz]',1,'MonitorDataHeader'));
    tr.appendChild(Cell('Phys [Hz]',2,'MonitorDataHeader'));
    tr.appendChild(Cell('MinB [Hz]',1,'MonitorDataHeader'));
    tr.appendChild(Cell('Lumi [Hz]',1,'MonitorDataHeader'));

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Now:',1,null));
    tr.appendChild(tab.l0Rate);
    tr.appendChild(tab.hltRate);
    tr.appendChild(Cell('Incl:', 1,'Text-Left'));
    tr.appendChild(tab.physicsRateIn);
    tr.appendChild(tab.minbRateIn);
    tr.appendChild(tab.LumiRateIn);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Run:',1,null));
    tr.appendChild(tab.l0RateRun);
    tr.appendChild(tab.hltRateRun);
    tr.appendChild(Cell('Excl:',1,'Text-Left'));
    tr.appendChild(tab.physicsRateEx);
    tr.appendChild(tab.minbRateEx);
    tr.appendChild(tab.LumiRateEx);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('TCK:',1,'MonitorDataHeader'));
    tab.trgConfig.colSpan=6;
    tr.appendChild(tab.trgConfig);

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.runType);
      provider.subscribeItem(this.runStart);
      provider.subscribeItem(this.nTriggers);
      provider.subscribeItem(this.hltNTriggers);
      provider.subscribeItem(this.l0Rate);
      provider.subscribeItem(this.l0RateRun);
      provider.subscribeItem(this.hltRate);
      provider.subscribeItem(this.hltRateRun);
      provider.subscribeItem(this.deadTime);
      provider.subscribeItem(this.deadTimeRun);
      provider.subscribeItem(this.veloPosition);
      
      provider.subscribeItem(this.bcmBeamPermit1);
      provider.subscribeItem(this.bcmBeamPermit2);
      provider.subscribeItem(this.bcmBeamPermit3);
      
      provider.subscribeItem(this.magnetField);
      provider.subscribeItem(this.magnetPolarity);
      provider.subscribeItem(this.hvState);
      
      provider.subscribeItem(this.figureOfMerit1);
      provider.subscribeItem(this.figureOfMerit2);
      provider.subscribeItem(this.figureOfMerit3);
      provider.subscribeItem(this.figureOfMerit4);

      provider.subscribeItem(this.instantLumi);
      provider.subscribeItem(this.interActionRate);

      provider.subscribeItem(this.physicsRateIn);
      provider.subscribeItem(this.physicsRateEx);
      provider.subscribeItem(this.LumiRateIn);
      provider.subscribeItem(this.LumiRateEx);
      provider.subscribeItem(this.minbRateIn);
      provider.subscribeItem(this.minbRateEx);
      provider.subscribeItem(this.trgConfig);
      return this;
    };
    return tab;
  };

  /** Build table with HT summary state of the LHCb detector
   *   
   * @author  M.Frank
   * @version 1.0
   */
  lhcb.widgets.LHCb_HT_header = function() {
    var c, tb, td, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
  
    tab.width     = '100%';
    tab.className = 'MonitorData';
    tb.className  = 'MonitorData';
  
    tr = document.createElement('tr');
    tr.appendChild(Cell('Detector HV Status',2,'MonitorDataHeader'));
    tb.appendChild(tr);
  
    tr = document.createElement('tr');
    tr.appendChild(c=Cell('LHCb LV & HV is:',1,null));    
    c.style.width  = '120px';
    this.hvState   = StyledItem('lbWeb.LHCb_LHC_HV.FSM.state',null,null);
    //this.hvState = StyledItem('lbWeb.LHCb_HV.FSM.state',null,null);
    this.hvState.conversion = function(val) {
      if ( val ) {
	var v=val.split('/',4);
	if ( v.length > 1 ) return v[1];
      }
      return val;
    };
    tr.appendChild(this.hvState);
    tb.appendChild(tr);
    /*
    tr = document.createElement('tr');
    tr.appendChild(c=Cell('Updated:',1,null));    
    c.style.width = '120px';
    this.hvStateUpdate = StyledItem('lbWeb.LHCb.HVStateUpdate',null,null);
    tr.appendChild(this.hvStateUpdate);
    tb.appendChild(tr);
    */
    tab.appendChild(tb);
    return tab;
  };

  /// Create Cell item containing the TTC prepulse information
  lhcb.widgets.rf2ttcPrepulses = function() {
    var item = StyledItem('lbWeb.RF2TTC/rf2ttc.Parameter.Readings.DIP.INJECTION_PREPULSE','Text-Right',null);
    item.conversion = function(v) { return (v=='TRUE') ? 'Yes' : 'No'; };
    return item;
  };

  /// Create Cell item containing the TTC clock status
  lhcb.widgets.rf2ttcStatus = function() {
    var item = StyledItem('lbWeb.RF2TTC/rf2ttc.Parameter.Settings.Errors.FREQ_STATUS',null,null);
    item.conversion = function(v) {  return (v=='TRUE') ? 'Clock OK' : 'Bad Clock'; };
    return item
  };

  /// Create Cell item containing the TTC clock state
  lhcb.widgets.rf2ttcState = function() {
    var item = StyledItem('lbWeb.RF2TTC/rf2ttc.State.RunState',null,null);
    item.data = 3;
    item.conversion = function(state) {
      this.data = state;
      if(state == 3 || state == 5)    //state Ready or Running
	return 'Ready';
      else if(state == 1 || state == 2) // Not Ready or Configuring
        return 'Not Ready';
      return 'Off';
    };
    return item
  };

  /// Create Cell item containing the TTC clock type. Requires the state for interpretation!
  lhcb.widgets.rf2ttcSource = function(st) {
    var item = StyledItem('lbWeb.RF2TTC/rf2ttc.Parameter.Settings.Selection.SELECT_SOURCE','Text-Right',null);
    item.State  = st;
    item.State.data = 3;
    item.conversion = function(source)  {
      var state = state = this.State.data;
      if(state == 3 || state == 5) {  //state Ready or Running
	if(source == 0)
	  return 'Int. LHCb clock'; 
	else if (source == 1)
          return 'Ext. LHC clock: locked on RF Ref'; 
	else if (source == 2)
          return 'Ext. LHC clock: locked on Beam 1'; 
	else if (source == 3)
          return 'Ext. LHC clock: locked on Beam 2';
      }
      else if(state == 1 || state == 2) // Not Ready or Configuring
        return 'Not configured';
      else if(state == 0)
        return 'Problem with the clock';
      else if(state == -1)
        return 'No Clock Monitoring';
      return 'System not used or PVSS project not connected';
    };
    return item;
  };

  lhcb.widgets.LHCStateSummary = function(options) {
    var tb, tr, cell, tab = document.createElement('table');
    tab.className  = 'MonitorPage';

    tb = document.createElement('tbody');
    tooltips.set(tb,'LHC state summary<br>Click to move to LHC summary information.');
    tb.onclick = function() { document.location = lhcb.constants.lhcb_display_url("lhc");};
    tb.className  = 'MonitorPage';
    tb.height    = '120px';
    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('LHC State',3,options.style));
      tb.appendChild(tr);
    }
    if ( options.legend ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('Mode',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Fill Number',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Energy',1,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    tab.lhcMode = StyledItem('lbWeb.LHCCOM/LHC.LHC.RunControl.MachineMode',null,null);
    tab.lhcFillNumber    = StyledItem('lbWeb.LHCCOM/LHC.LHC.RunControl.FillNumber',null,null);
    tab.lhcEnergy        = StyledItem('lbWeb.LHCCOM/LHC.LHC.Beam.Energy',null,'%7.1f GeV');
    tab.lhcEnergy.conversion = function(data) {      return (data*120.0)/1000.0;    };
    tr = document.createElement('tr');
    tr.appendChild(tab.lhcMode);
    tr.appendChild(tab.lhcFillNumber);
    tr.appendChild(tab.lhcEnergy);
    tb.appendChild(tr);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.lhcMode);
      provider.subscribeItem(this.lhcFillNumber);
      provider.subscribeItem(this.lhcEnergy);
    };
    return tab;
  };


  lhcb.widgets.ClockSummary = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr = document.createElement('tr');
    var cell = Cell('LHC clock:',null,'MonitorDataHeader');

    tooltips.set(tab,'TTC clock information<br>Click to see LHC status');
    tab.onclick = function() { document.location = lhcb.constants.lhcb_display_url("lhc");};
    tab.lhcPrepulses   = lhcb.widgets.rf2ttcPrepulses();
    tab.lhcClockState  = lhcb.widgets.rf2ttcState();
    tab.lhcClock       = lhcb.widgets.rf2ttcSource(tab.lhcClockState);
    tab.lhcStatus      = lhcb.widgets.rf2ttcStatus();

    tab.className = tb.className   = 'MonitorPage';
    if ( options.style ) {
      tr.appendChild(Cell('LHC Clock Status',4,options.style));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }

    tr.appendChild(cell);
    cell.style.width = '25%';
    tr.appendChild(tab.lhcClock);
    tab.lhcClock.colSpan = 5;
    tab.style.textAlign = 'left';
    tb.appendChild(tr);

    tr = document.createElement('tr');
    tr.appendChild(cell=Cell('Clock state:',null,'MonitorDataHeader'));
    tr.appendChild(tab.lhcClockState);
    tab.lhcClockState.style.textAlign = 'left';
    tr.appendChild(cell=Cell('/',null,null));
    tr.appendChild(tab.lhcStatus);
    tr.appendChild(cell=Cell('Prepulses:',null,'MonitorDataHeader'));
    cell.style.width = '15%';
    tr.appendChild(tab.lhcPrepulses);
    tab.lhcPrepulses.style.width = '15%';
    tb.appendChild(tr);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.lhcClock);
      provider.subscribeItem(this.lhcStatus);
      provider.subscribeItem(this.lhcClockState);
      provider.subscribeItem(this.lhcPrepulses);
    };
    return tab;
  };

  lhcb.widgets.RateSummary = function(options) {

    var tb, td, tr, tab = document.createElement('table');
    var sys = options.system;
    tb = document.createElement('tbody');

    tab.className  = tb.className = 'MonitorPage';
    tab.width      = '100%';
    tb.cellSpacing = 0;
    tb.cellPadding = 0;

    tab.runType        = StyledItem('lbWeb.'+sys+'_RunInfo.general.runType',     null,null);
    tab.nTriggers      = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.nTriggers',       null,null);
    tab.hltNTriggers   = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.hltNTriggers',null,null);
    tab.l0Rate         = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.triggerRate',     null,'%8.2f Hz');
    tab.l0RateRun      = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.runTriggerRate',  null,'%8.2f Hz');
    tab.hltRate        = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.hltRate',     null,'%8.2f Hz');
    tab.hltRateRun     = StyledItem('lbWeb.'+sys+'_RunInfo.HLTFarm.runHltRate',  null,'%8.2f Hz');
    tab.deadTime       = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.deadTime',        null,'%8.2f %%');
    tab.deadTimeRun    = StyledItem('lbWeb.'+sys+'_RunInfo.TFC.runDeadTime',     null,'%8.2f %%');
    tab.interActionRate= StyledItem('lbWeb.LHCCOM/LHC.LHCb.Internal.TriggerRates.TrgRateLumi_GP', null, '%7.1f Hz');
    tab.instantLumi    = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Internal.Luminosity.LumiInst_GP', null, '%7.2f Hz/&mu;b');

    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Data taking rates and instantaneous luminosity',5,options.style));
    }

    // Run status
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Activity:',null,'MonitorDataHeader'));
    tr.appendChild(tab.runType);
    tab.runType.colSpan = 4;
    tab.runType.style.textAlign = 'left';

    // Luminosity and interaction information
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Interaction rate:',null,'MonitorDataHeader'));
    tr.appendChild(tab.interActionRate);
    tr.appendChild(Cell('Inst.Lumi:',1,'MonitorDataHeader'));
    tr.appendChild(tab.instantLumi);

    // Event processing information
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('L0 Events:',null,'MonitorDataHeader'));
    tr.appendChild(tab.nTriggers);
    tr.appendChild(Cell('accepted:',1,'MonitorDataHeader'));
    tr.appendChild(tab.hltNTriggers);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('',1,null));
    tr.appendChild(Cell('Now',2,'MonitorDataHeader'));
    tr.appendChild(Cell('Run',2,'MonitorDataHeader'));

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('L0 rate:',1,'MonitorDataHeader'));
    tab.l0Rate.colSpan = 2;
    tab.l0RateRun.colSpan = 2;
    tr.appendChild(tab.l0Rate);
    tr.appendChild(tab.l0RateRun);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('HLT rate:',1,'MonitorDataHeader'));
    tab.hltRate.colSpan = 2;
    tab.hltRateRun.colSpan = 2;
    tr.appendChild(tab.hltRate);
    tr.appendChild(tab.hltRateRun);

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('Dead-time:',null,'MonitorDataHeader'));
    tab.deadTime.colSpan = 2;
    tab.deadTimeRun.colSpan = 2;
    tr.appendChild(tab.deadTime);
    tr.appendChild(tab.deadTimeRun);

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.runType);
      provider.subscribeItem(this.nTriggers);
      provider.subscribeItem(this.hltNTriggers);
      provider.subscribeItem(this.l0Rate);
      provider.subscribeItem(this.l0RateRun);
      provider.subscribeItem(this.hltRate);
      provider.subscribeItem(this.hltRateRun);
      provider.subscribeItem(this.deadTime);
      provider.subscribeItem(this.deadTimeRun);
      provider.subscribeItem(this.instantLumi);
      provider.subscribeItem(this.interActionRate);
      return this;
    };
    return tab;
  };

  lhcb.widgets.MagnetSummary = function(options) {
    var tb, tr, tab = document.createElement('table');
    tab.className  = 'MonitorPage';
    
    tb = document.createElement('tbody');
    tooltips.set(tb,'Magnet summary<br>Click to move to magnet page.');
    tb.onclick = function() { document.location = lhcb.constants.lhcb_display_url("magnet");};
    tb.className  = 'MonitorPage';
    tb.height    = '120px';
    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Magnet Status',3,options.style));
    }
    if ( options.legend ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Set Current',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Measured Current',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Polarity',1,'MonitorDataHeader'));
    }
    tb.appendChild(tr=document.createElement('tr'));
    tab.magSet      = StyledItem('lbWeb.LbMagnet.SetCurrent',null,'%7.0f A');
    tab.magRead     = StyledItem('lbWeb.LbMagnet.Current',null,'%7.0f A');
    tab.magPolarity = StyledItem('lbWeb.LbMagnet.Polarity',null,null);
    tab.magPolarity.conversion = function(data) {      return data>0 ? 'Down' : 'Up';    };
    tr = document.createElement('tr');
    tr.appendChild(tab.magSet);
    tr.appendChild(tab.magRead);
    tr.appendChild(tab.magPolarity);

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.magSet);
      provider.subscribeItem(this.magRead);
      provider.subscribeItem(this.magPolarity);
    };
    return tab;
  };

  lhcb.widgets.CoolingSummary = function(options) {
    var c, tb, td, tr, tab = document.createElement('table');
    tb = document.createElement('tbody');
    tab.className = tb.className   = 'MonitorPage';

    tooltips.set(tab,'Cooling information summary<br>for various subdetectors');    
    // Velo position
    tab.itCoolingAlarms    = StyledItem('lbWeb.CaV/ItPlant.Actual.alarm', null, null);
    tab.otCoolingAlarms    = StyledItem('lbWeb.CaV/OtPlant.Actual.alarm', null, null);
    tab.ttCoolingAlarms    = StyledItem('lbWeb.CaV/TtPlant.Actual.alarm', null, null);
    tab.richCoolingAlarms  = StyledItem('lbWeb.CaV/RichPlant.Actual.alarm', null, null);

    tab.itCoolingFaults    = StyledItem('lbWeb.CaV/ItPlant.Actual.fault', null, null);
    tab.otCoolingFaults    = StyledItem('lbWeb.CaV/OtPlant.Actual.fault', null, null);
    tab.ttCoolingFaults    = StyledItem('lbWeb.CaV/TtPlant.Actual.fault', null, null);
    tab.richCoolingFaults  = StyledItem('lbWeb.CaV/RichPlant.Actual.fault', null, null);
    /*
    tab.itCoolingStatus    = StyledItem('lbWeb.CaV/ItPlant.Actual.status', null, null);
    tab.otCoolingStatus    = StyledItem('lbWeb.CaV/OtPlant.Actual.status', null, null);
    tab.ttCoolingStatus    = StyledItem('lbWeb.CaV/TtPlant.Actual.status', null, null);
    tab.richCoolingStatus  = StyledItem('lbWeb.CaV/RichPlant.Actual.status', null, null);
    */
    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Cooling Plant Status',5,options.style));
    }
    tr = document.createElement('tr');
    tb.appendChild(tr);
    if ( options.legend ) {
      tr.appendChild(c=Cell('Cooling',1,'MonitorDataHeader'));
      c.style.width = '25%';
    }
    tr.appendChild(Cell('IT',1,'MonitorDataHeader'));
    tr.appendChild(Cell('TT',1,'MonitorDataHeader'));
    tr.appendChild(Cell('OT',1,'MonitorDataHeader'));
    tr.appendChild(Cell('Rich',1,'MonitorDataHeader'));

    tb.appendChild(tr=document.createElement('tr'));
    if ( options.legend ) tr.appendChild(Cell('Alarms',1,'MonitorDataHeader'));

    tr.appendChild(tab.itCoolingAlarms);
    tr.appendChild(tab.ttCoolingAlarms);
    tr.appendChild(tab.otCoolingAlarms);
    tr.appendChild(tab.richCoolingAlarms);

    tb.appendChild(tr=document.createElement('tr'));
    if ( options.legend ) tr.appendChild(Cell('Faults',1,'MonitorDataHeader'));

    tr.appendChild(tab.itCoolingFaults);
    tr.appendChild(tab.ttCoolingFaults);
    tr.appendChild(tab.otCoolingFaults);
    tr.appendChild(tab.richCoolingFaults);

    /*
    tb.appendChild(tr=document.createElement('tr'));
    if ( options.legend ) tr.appendChild(Cell('Status',1,'MonitorDataHeader'));

    tr.appendChild(tab.itCoolingStatus);
    tr.appendChild(tab.ttCoolingStatus);
    tr.appendChild(tab.otCoolingStatus);
    tr.appendChild(tab.richCoolingStatus);
    */
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.itCoolingAlarms);
      provider.subscribeItem(this.ttCoolingAlarms);
      provider.subscribeItem(this.otCoolingAlarms);
      provider.subscribeItem(this.richCoolingAlarms);
      provider.subscribeItem(this.itCoolingFaults);
      provider.subscribeItem(this.ttCoolingFaults);
      provider.subscribeItem(this.otCoolingFaults);
      provider.subscribeItem(this.richCoolingFaults);
      //provider.subscribeItem(this.itCoolingStatus);
      //provider.subscribeItem(this.ttCoolingStatus);
      //provider.subscribeItem(this.otCoolingStatus);
      //provider.subscribeItem(this.richCoolingStatus);
    };
    return tab;
  };

  lhcb.widgets.BackgroundSummary = function(options) {
    var c, tr;
    var tab = document.createElement('table');
    var tb  = document.createElement('tbody');

    tooltips.set(tab,'Background summary<br>Click to access BCM information');
    tab.onclick = function() { document.location = lhcb.constants.lhcb_display_url("bcm&sensors=1");};
    tab.className = tb.className = 'MonitorPage';

    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(c=Cell('Background Status and Beam Permits:',7,options.style));
      c.style.width='100%';
    }

    tab.bcmBeamPermit1 = StyledItem('lbWeb.BCM_Interface.BeamPermit.getStatus',null,null);
    tab.bcmBeamPermit2 = StyledItem('lbWeb.BCM_Interface.InjPermit1.getStatus',null,null);
    tab.bcmBeamPermit3 = StyledItem('lbWeb.BCM_Interface.InjPermit2.getStatus',null,null);
    tb.appendChild(tr=document.createElement('tr'));
    if ( options.legend ) {
      tr.appendChild(c=Cell('Permits',1,'MonitorDataHeader'));
      c.style.width = '25%';
    }
    tr.appendChild(tab.bcmBeamPermit1);
    tab.bcmBeamPermit1.colSpan = 2;
    tr.appendChild(tab.bcmBeamPermit2);
    tab.bcmBeamPermit2.colSpan = 2;
    tr.appendChild(tab.bcmBeamPermit3);
    tab.bcmBeamPermit3.colSpan = 2;

    tab.figureOfMerit1 = StyledItem('lbWeb.BCM_DP_S0.RS2_REL',             null, '%7.3f');
    tab.figureOfMerit2 = StyledItem('lbWeb.BCM_DP_S0.RS32_REL',            null, '%7.3f');
    tab.figureOfMerit3 = StyledItem('lbWeb.BCM_DP_S1.RS2_REL',             null, '%7.3f');
    tab.figureOfMerit4 = StyledItem('lbWeb.BCM_DP_S1.RS32_REL',            null, '%7.3f');
    tb.appendChild(tr=document.createElement('tr'));
    if ( options.legend ) {
      tr.appendChild(Cell('FoM',null,'MonitorDataHeader'));
    }
    tr.appendChild(Cell('S0.RS2/32:',1,'MonitorDataHeader'));
    tr.appendChild(tab.figureOfMerit1);
    tr.appendChild(tab.figureOfMerit2);
    tr.appendChild(Cell('S1.RS2/32:',1,'MonitorDataHeader'));
    tr.appendChild(tab.figureOfMerit3);
    tr.appendChild(tab.figureOfMerit4);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.bcmBeamPermit1);
      provider.subscribeItem(this.bcmBeamPermit2);
      provider.subscribeItem(this.bcmBeamPermit3);
      
      provider.subscribeItem(this.figureOfMerit1);
      provider.subscribeItem(this.figureOfMerit2);
      provider.subscribeItem(this.figureOfMerit3);
      provider.subscribeItem(this.figureOfMerit4);
    };
    return tab;
  };

  lhcb.widgets.SafetySummary = function(options) {
    var tr, cell, tab = document.createElement('table');
    var tb = document.createElement('tbody');

    tab.className =  tb.className = 'MonitorPage';
    tooltips.set(tb,'Safety status of the subdetectors');
    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Safety',3,options.style));
    }
    if ( options.legend ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Subdetector',1,'MonitorDataHeader'));
      tr.appendChild(Cell('State',1,'MonitorDataHeader'));
      tr.appendChild(Cell('',1,null));
    }
    var opt = (!options.legend && !options.style) ? ' Safety Status' : '';
    if ( options.all || options.rich ) tab.richSafety = FSMItem('lbWeb.RICH_Safety',options.logger,true);
    if ( options.all || options.muon ) tab.muonSafety = FSMItem('lbWeb.MUON_Safety',options.logger,true);
    if ( options.all || options.tt   ) tab.ttSafety   = FSMItem('lbWeb.TT_Safety',options.logger,true);
    if ( options.all || options.it   ) tab.itSafety   = FSMItem('lbWeb.IT_Safety',options.logger,true);
    if ( options.all || options.tt   ) tb.appendChild(lhcb.widgets.mkFSMitem1(tab.ttSafety,'TT'+opt));
    if ( options.all || options.it   ) tb.appendChild(lhcb.widgets.mkFSMitem1(tab.itSafety,'IT'+opt));
    if ( options.all || options.rich ) tb.appendChild(lhcb.widgets.mkFSMitem1(tab.richSafety,'RICH'+opt));
    if ( options.all || options.muon ) tb.appendChild(lhcb.widgets.mkFSMitem1(tab.muonSafety,'MUON'+opt));
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      if (this.ttSafety)   provider.subscribeItem(this.ttSafety);
      if (this.itSafety)   provider.subscribeItem(this.itSafety);
      if (this.richSafety) provider.subscribeItem(this.richSafety);
      if (this.muonSafety) provider.subscribeItem(this.muonSafety);
    };

    return tab;
  };

  /** Online widgets. RICH gas pressures
   *
   *  @author M.Frank
   */
  lhcb.widgets.RICHPressures = function(options) {
    var tb, tr, cell, tab = document.createElement('table');

    tb = document.createElement('tbody');
    tooltips.set(tb,'Rich parameters');
    tab.className =  tb.className = 'MonitorPage';

    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Rich1 Gas',2,options.style));
      tr.appendChild(Cell('Rich2 Gas',2,options.style));
    }
    if ( options.legend ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Pressure',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Temperature',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Pressure',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Temperature',1,'MonitorDataHeader'));
    }
    if ( options.tips    ) tooltips.set(tb,options.tips);
    if ( options.onclick ) tb.onclick = options.onclick; 

    tb.appendChild(tr);
    tab.rich1Pressure = StyledItem('lbWeb.LHCb_RunInfoCond.RICH1.R1HltGasParameters.Pressure',null,'%7.1f hPa');
    tab.rich1Temp     = StyledItem('lbWeb.LHCb_RunInfoCond.RICH1.R1HltGasParameters.Temperature',null,'%7.1f K');
    tab.rich2Pressure = StyledItem('lbWeb.LHCb_RunInfoCond.RICH2.R2HltGasParameters.Pressure',null,'%7.2f hPa');
    tab.rich2Temp     = StyledItem('lbWeb.LHCb_RunInfoCond.RICH2.R2HltGasParameters.Temperature',null,'%7.1f K');

    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(tab.rich1Pressure);
    tr.appendChild(tab.rich1Temp);
    tr.appendChild(tab.rich2Pressure);
    tr.appendChild(tab.rich2Temp);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.rich1Pressure);
      provider.subscribeItem(this.rich1Temp);
      provider.subscribeItem(this.rich2Pressure);
      provider.subscribeItem(this.rich2Temp);
    };
    return tab;
  };
  lhcb.widgets.rich = new Object();
  lhcb.widgets.rich.Pressures = lhcb.widgets.RICHPressures;

  /** Online widgets. HV Summary for subdetector(s) from ECS project
   *
   *  @author M.Frank
   */
  lhcb.widgets.HVSummary = function(logger) {
    var tb, tr, cell, tab = document.createElement('table');
    tab.className  = 'MonitorPage';
    tab.logger = logger;

    tab.addState = function(nam,split,m,q) {
      var cell,tr = document.createElement('tr');
      tr.appendChild(cell=Cell(nam,1,null));
      cell.style.width='120px';
      this[m+'LHC'+nam]   = FSMItem('lbWeb.'+nam+'_LHC_'+m,this.logger,true);
      this[m+'LHCRQ'+nam] = StyledItem('lbWeb.'+nam+'_LHC_'+m+'_Comm.PrevLhcState',null,null);
      tr.appendChild(this[m+'LHC'+nam]);      
      tr.appendChild(this[m+'LHCRQ'+nam]);
      
      if ( split ) {
	this[m+'A'+nam] = FSMItem('lbWeb.'+nam+'A_'+q,this.logger,true);
	this[m+'C'+nam] = FSMItem('lbWeb.'+nam+'C_'+q,this.logger,true);
	tr.appendChild(this[m+'A'+nam]);
	tr.appendChild(this[m+'C'+nam]);
      }
      else {
	this[m+nam] = FSMItem('lbWeb.'+nam+'_'+q,this.logger,true);
	tr.appendChild(this[m+nam]);
      }
      tr.appendChild(this[m+'LHC'+nam].lock);
      this[m+'LHC'+nam].lock.className = null;
      tooltips.set(tr,'HT state of the '+nam+' subdetector');
      return tr;
    };

    tab.addHV = function(tb) {
      tb.appendChild(this.addState('VELO',  true, 'HV', 'HV'));
      tb.appendChild(this.addState('TT',   false, 'HV', 'HV'));
      tb.appendChild(this.addState('IT',   false, 'HV', 'HV'));
      tb.appendChild(this.addState('OT',    true, 'HV', 'HV'));
      tb.appendChild(this.addState('RICH1',false, 'HV', 'HV'));
      tb.appendChild(this.addState('RICH2',false, 'HV', 'HV'));
      tb.appendChild(this.addState('PRS',  false, 'HV', 'HV'));
      tb.appendChild(this.addState('ECAL', false, 'HV', 'HV'));
      tb.appendChild(this.addState('HCAL', false, 'HV', 'HV'));
      tb.appendChild(this.addState('MUON',  true, 'HV', 'HV'));
    };

    tab.addLV = function(tb) {
      tb.appendChild(this.addState('VELO',   true, 'LV', 'DCS_Local'));
      tb.appendChild(this.addState('TT',    false, 'LV', 'DCS_Local'));
      tb.appendChild(this.addState('IT',    false, 'LV', 'DCS_Local'));
      tb.appendChild(this.addState('RICH1', false, 'LV', 'DCS_Local'));
      tb.appendChild(this.addState('RICH2', false, 'LV', 'DCS_Local'));
    };

    tab.buildTable = function(options) {
      var tb, tr, cell;
      tb = document.createElement('tbody');
      tb.className = 'MonitorPage';
      tb.height    = '120px';
      if ( options.split ) this.split = true;
      tooltips.set(tb,'HT summary of LHCb');
      if ( options.style ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('Voltages',6,options.style));
      }
      if ( options.top ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('System',1,'MonitorDataHeader'));
	tr.appendChild(Cell('State',1,'MonitorDataHeader'));
	tr.appendChild(Cell('Requested',1,'MonitorDataHeader'));

	this.hvState   = FSMItem('lbWeb.LHCb_LHC_HV',this.logger,true);
	this.hvRequest = StyledItem('lbWeb.LHCb_LHC_HV_Info.requestedState',null,null);
	
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('LHCb HV &amp; LV',1,'MonitorDataHeader'));
	tr.appendChild(this.hvState);
	tr.appendChild(this.hvRequest);
	tr.appendChild(this.hvState.lock);
	this.hvState.lock.className = null;
	this.appendChild(tb);
      }

      tb = document.createElement('tbody');
      tooltips.set(tab,'High voltage state of the subdetectors');
      tb.className  = 'MonitorPage';
      tb.height    = '120px';
      if ( options.hv ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('High Voltage',6,'Arial12pt'));
      }
      if ( options.hv_legend ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(cell=Cell('Subdetector',1,'MonitorDataHeader'));
	tr.appendChild(cell=Cell('State',1,'MonitorDataHeader'));
	tr.appendChild(cell=Cell('Request',1,'MonitorDataHeader'));
	cell.style.width = '20%';
	tr.appendChild(Cell(this.split ? 'HV State Side A / C' : 'HV State',2,'MonitorDataHeader'));
      }
      this.addHV(tb);
      this.appendChild(tb);
      
      tb = document.createElement('tbody');
      tooltips.set(tb,'Low voltage state of the subdetectors');
      tb.className  = 'MonitorPage';
      tb.height    = '120px';
      if ( options.lv ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('Low Voltage',6,'Arial12pt'));
      }
      if ( options.lv_legend ) {
	tb.appendChild(tr=document.createElement('tr'));
	tr.appendChild(Cell('Subdetector',1,'MonitorDataHeader'));
	tr.appendChild(Cell('State',1,'MonitorDataHeader'));
	tr.appendChild(cell=Cell('Request',1,'MonitorDataHeader'));
	tr.appendChild(Cell('LV State Side A / Side C',2,'MonitorDataHeader'));
      }
      this.addLV(tb);
      this.appendChild(tb);
      return this;
    };

    tab.build = function(header_style) {
      return this.buildTable({style:header_style,top:true,hv:true,hv_legend:true,lv:true,lv_legend:true});
    };

    tab.subscribe_j = lhcb.widgets.subscribeItem;

    tab.subscribe_i = function(provider,typ,nam) {
      this.subscribe_j(provider, this[typ+'LHC'+nam]);
      this.subscribe_j(provider, this[typ+'LHCRQ'+nam]);
      this.subscribe_j(provider, this[typ+nam]);
      this.subscribe_j(provider, this[typ+'A'+nam]);
      this.subscribe_j(provider, this[typ+'C'+nam]);
    };
    tab.subscribe = function(provider) {
      this.subscribe_i(provider, 'HV', 'VELO');
      this.subscribe_i(provider, 'HV', 'TT');
      this.subscribe_i(provider, 'HV', 'IT');
      this.subscribe_i(provider, 'HV', 'OT');
      this.subscribe_i(provider, 'HV', 'RICH1');
      this.subscribe_i(provider, 'HV', 'RICH2');
      this.subscribe_i(provider, 'HV', 'PRS');
      this.subscribe_i(provider, 'HV', 'ECAL');
      this.subscribe_i(provider, 'HV', 'HCAL');
      this.subscribe_i(provider, 'HV', 'MUON');

      this.subscribe_i(provider, 'LV', 'VELO');
      this.subscribe_i(provider, 'LV', 'TT');
      this.subscribe_i(provider, 'LV', 'IT');
      this.subscribe_i(provider, 'LV', 'RICH1');
      this.subscribe_i(provider, 'LV', 'RICH2');
      if ( this['hvState']  )  provider.subscribeItem(this.hvState);
      if ( this['hvRequest'] ) provider.subscribeItem(this.hvRequest);
    };

    return tab;
  };


  /** SystemSummary class
   *
   *  @author M.Frank
   */
  lhcb.widgets.SystemSummary = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr = document.createElement('tr');
    var split = options.split;
    var sys = options.system;
    var cell;
    tab.className = tb.className   = 'MonitorPage';
    tab.style.height = tb.style.height = '100%';
    if ( options.style ) {
      var tit = options.title;
      if ( !tit ) tit = 'VELO Subsystem State';
      if ( options.rowing ) 
	tr.appendChild(cell=Cell(tit,5,options.style));
      else 
	tr.appendChild(Cell(tit,4,options.style));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    if ( options.legend ) {
      if ( options.rowing ) tr.appendChild(cell=Cell('',1,null));
      if ( split ) {
	tr.appendChild(cell=Cell('Side A',2,'MonitorDataHeader'));
	tr.appendChild(cell=Cell('Side C',2,'MonitorDataHeader'));
	tb.appendChild(tr);
      }
    }
    tab.subscriptions=[];
    tab.addSystem = function(system,name) {
      tr = document.createElement('tr');
      if ( options.rowing ) tr.appendChild(cell=Cell(name,1,'MonitorDataHeader'));
      if ( split ) {
	this[name+'A'] = FSMItem('lbWeb.'+system+'A_'+name,options.logger,true);
	this[name+'C'] = FSMItem('lbWeb.'+system+'C_'+name,options.logger,true);
	tr.appendChild(this[name+'A']);
	tr.appendChild(this[name+'A'].lock);
	this[name+'A'].style.width = '30%';
	tr.appendChild(this[name+'C']);
	tr.appendChild(this[name+'C'].lock);
	this[name+'C'].style.width = '30%';
	this.subscriptions.push(this[name+'A']);
	this.subscriptions.push(this[name+'C']);
      }
      else {
	this[name]  = FSMItem('lbWeb.'+system+'_'+name,options.logger,true);
	tr.appendChild(this[name]);
	tr.appendChild(this[name].lock);
	this[name].style.width = '30%';
	this.subscriptions.push(this[name]);
      }
      return tr;
    };
    for(var i=0; i<options.items.length;++i)
      tb.appendChild(tab.addSystem(sys,options.items[i]));
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      for(var i=0; i<this.subscriptions.length;++i)
	provider.subscribeItem(this.subscriptions[i]);
    };
    return tab;
  };

  /** Online widgets. DAQ efficiency
   *
   *  @author M.Frank
   */
  lhcb.widgets.DAQEfficiency = function(options) {
    var tr, tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var type = options.efficiency_type;
    if ( !type ) type = 'Lumi';
    tab.className  = tb.className = 'MonitorPage';
    
    tooltips.set(tb,'Run efficiency summary normalized to<br> '+type+' of datataking during this fill.');
    tb.height    = '120px';
    if ( options.style ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Efficiency normalized to '+type,3,options.style));
    }
    if ( options.legend ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('HV Ready',   1,'MonitorDataHeader'));
      tr.appendChild(Cell('VELO IN',    1,'MonitorDataHeader'));
      tr.appendChild(Cell('DAQ stopped',1,'MonitorDataHeader'));
      tr.appendChild(Cell('dead time',  1,'MonitorDataHeader'));
    }
    tab.hv      = StyledItem('lbWeb.LHCbEfficiency.Results'+type+'.HV',null,'%7.2f %%');
    tab.velo    = StyledItem('lbWeb.LHCbEfficiency.Results'+type+'.VELO',null,'%7.2f %%');
    tab.daq     = StyledItem('lbWeb.LHCbEfficiency.Results'+type+'.DAQ',null,'%7.2f %%');
    tab.dead    = StyledItem('lbWeb.LHCbEfficiency.Results'+type+'.DAQLiveTime',null,'%7.2f %%');
    tab.total   = StyledItem('lbWeb.LHCbEfficiency.'+type+'Total',null,'%7.2f %%');
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(tab.hv);
    tr.appendChild(tab.velo);
    tr.appendChild(tab.daq);
    tr.appendChild(tab.dead);
    tr.appendChild(tab.total);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.hv);
      provider.subscribeItem(this.velo);
      provider.subscribeItem(this.daq);
      provider.subscribeItem(this.dead);
      provider.subscribeItem(this.total);
    };
    return tab;
  };

  /** Online widgets. Base class to show subdetector FSM groups
   *
   *  @author M.Frank
   */
  lhcb.widgets.SubdetectorSystem = function(options) {
    var tab = document.createElement('table');
    tab.subscriptions=[];
    tab.logger = options.logger;
    tab.className = 'MonitorPage';

    tab.addFSMRow = function(names) {
      var tr = document.createElement('tr');
      for(var i=0; i<names.length; ++i) {
	var name = names[i];
	if ( name ) {
	  var cell = Cell(name,1,'MonitorDataHeader');
	  var item = FSMItem('lbWeb.'+name,this.logger,true);
	  this[name] = item;
	  tr.appendChild(cell);
	  tr.appendChild(item);
	  tr.appendChild(item.lock);
	  this.subscriptions.push(item);
	}
	else {
	  tr.appendChild(Cell('',3,null));
	}
      }
      return tr;
    };

    tab.body = function(title) {
      var tb = document.createElement('tbody');
      tb.className   = 'MonitorPage';
      if ( options.style ) {
	var tr = document.createElement('tr');
	tr.appendChild(Cell(title,6,options.style));
	tb.appendChild(tr);
      }
      this.appendChild(tb);
      return tb;
    };

    tab.subscribe = function(provider) {
      for(var i=0; i<this.subscriptions.length;++i)
	provider.subscribeItem(this.subscriptions[i]);
    };
    
    return tab;
  };


  lhcb.widgets.SubdetectorPage = function(name, msg) {
    var table       = document.createElement('table');
    table.body      = document.createElement('tbody');
    table.system    = name;
    table.title     = name;
    table.messages  = msg;
    table.provider  = null;
    table.logger    = null;
    if ( table.system.indexOf(' ')>0 ) table.system = name.substr(0,name.indexOf(' '));
    table.add = function() {
      var td = document.createElement('td'), tr = document.createElement('tr');
      td.setAttribute('colSpan',2);
      tr.appendChild(td);
      this.body.appendChild(tr);
      return td;
    };
    table.className = table.body.className = 'MonitorOuterFrame';
    table.display = table.add();
    table.logDisplay = table.add();
    table.appendChild(table.body);
    table.subscriptions = new Array();

    table.hvSummary = function(options) {
      var tab = lhcb.widgets.HVSummary(this.logger);
      tab.split = true;
      if ( options.nosplit ) tab.split = false;
      tab.system = this.system;
      if ( options.system ) tab.system = options.system;
      tab.addHV = function(tb) {};
      tab.addLV = function(tb) {};
      if ( options.hv )  {
	tab.addHV = function(tb) { 
	  if ( this.system instanceof Array ) {
	    for(var i=0; i<this.system.length; ++i)
	      tb.appendChild(this.addState(this.system[i], this.split, 'HV', 'HV'));
	  }
	  else {
	    tb.appendChild(this.addState(this.system, this.split, 'HV', 'HV'));
	  }
	};
      }
      if ( options.lv )  {
	tab.addLV = function(tb) { 
	  if ( this.system instanceof Array ) {
	    for(var i=0; i<this.system.length; ++i)
	      tb.appendChild(this.addState(this.system[i], this.split, 'LV', 'DCS_Local'));
	  }
	  else {
	    tb.appendChild(this.addState(this.system, this.split, 'LV', 'DCS_Local'));
	  }
	};
      }
      return tab.buildTable({style:'Arial12pt',top:false,hv:options.hv,hv_legend:options.hv,lv:options.lv});
    };

    table.addRow = function(items) {
      var mx=1, tb = this.tbody, tr, td, item;
      tb.appendChild(tr=document.createElement('tr'));
      for(var j=0; j<items.length; ++j) {
	if ( items[j] instanceof Array ) {
	  if ( items[j].length>mx ) mx = items[j].length;
	} 
      }
      for(var i=0; i<items.length; ++i) {
	tr.appendChild(td=document.createElement('td'));
	td.style.verticalAlign = 'top';
	if ( items[i] instanceof Array ) {
	  for(var k=0, len=items[i].length; k<len; ++k) {
	    td.appendChild(items[i][k]);
	    this.subscriptions.push(items[i][k]);
	    if ( k == len-1 ) {
	      td.style.verticalAlign = 'bottom';
	      continue;
	    }
	    tb.appendChild(tr=document.createElement('tr'));
	    tr.appendChild(td=document.createElement('td'));
	    td.style.verticalAlign = 'top';
	  }
	} 
	else {
	  td.rowSpan = mx;
	  td.appendChild(items[i]);
	  this.subscriptions.push(items[i]);
	}
      }
    };

    /// Empty placeholder
    table.attachWidgets = function() {  };
    table.beforeComment = function() {  };

    table.build = function(options) {
      var tab = document.createElement('table');
      var tb = document.createElement('tbody');
      var t1, tb1, tr1, td1, cell, d = new Date();

      tab.className = tb.className = 'MonitorInnerFrame';

      this.heading = document.createElement('tr');
      this.heading.appendChild(td=document.createElement('td'));
      td.style.width = '100%';
      td.colSpan = 3;
      td.appendChild(t1=document.createElement('table'));
      t1.appendChild(tb1=document.createElement('tbody'));
      t1.width  = '100%';

      tb1.appendChild(tr1=document.createElement('tr'));
      tr1.appendChild(td1=document.createElement('td'));
      if ( options.logo ) {
	td1.appendChild(cell=Cell(options.logo,null,null));
	if ( options.logo_url ) {
	  cell.url = options.logo_url;
	  cell.onclick = function() { document.location = this.url; };
	}
      }
      tr1.appendChild(this.head_date=Cell(d.toString(),1,'MonitorTinyHeader'));
      this.head_date.id = 'current_time';
      this.head_date.textAlign = 'right';
      cell = Cell(this.title+'&nbsp;'+lhcb_online_picture(),1,'MonitorBigHeader');
      cell.style.textAlign = 'right';
      cell.onclick = function() { document.location = "http://lhcb.cern.ch";};
      tr1.appendChild(cell);
      if ( options.tips ) {
	tooltips.set(this.heading,options.tips);
      }
      tb.appendChild(this.heading);

      this.timerHandler = function() {document.getElementById('current_time').innerHTML = (new Date()).toString(); };
      setInterval(this.timerHandler,2000);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(td=document.createElement('td'));
      td.style.width = '55%';
      td.appendChild(this.lhc_header=lhcb.widgets.LHC_header());
      tr.appendChild(td=document.createElement('td'));
      td.appendChild(this.run_header=lhcb.widgets.Det_Run_header('LHCb',this.logger));
      td.width = '45%';

      this.lhc_header.style.height = '60px';
      this.run_header.style.height = '60px';
      this.subscriptions.push(this.lhc_header);
      this.subscriptions.push(this.run_header);


      tb.appendChild(tr=document.createElement('tr'));
      // Left half of the display
      tr.appendChild(td=document.createElement('td'));
      td.appendChild(t1=document.createElement('table'));
      t1.appendChild(this.left=document.createElement('tbody'));
      t1.className = this.left.className = 'MonitorPage';
      t1.style.border = this.left.style.border = 'none';
      this.left.style.verticalAlign = 'top';

      // Right hand of the display
      tr.appendChild(td=document.createElement('td'));
      td.appendChild(t1=document.createElement('table'));
      t1.appendChild(this.right=document.createElement('tbody'));
      t1.className = this.right.className = 'MonitorPage';
      t1.style.border = this.right.style.border = 'none';
      this.right.style.verticalAlign = 'top';

      this.left.addItem = function(item) {
	var tr1, td1;
	this.appendChild(tr1=document.createElement('tr'));
	tr1.appendChild(td1=document.createElement('td'));
	td1.style.verticalAlign = 'top';
	td1.appendChild(item);
	this.parent.subscriptions.push(item);
	return td1;
      };
      this.left.addSpacer = function(height) {
	var tr1, td1;
	this.appendChild(tr1=document.createElement('tr'));
	tr1.appendChild(td1=document.createElement('td'));
	if ( height ) td1.style.height=height;
	return td1;
      };
      this.right.addItem = this.left.addItem;
      this.right.addSpacer = this.left.addSpacer;
      this.left.parent = this.right.parent = this;

      this.tbody = tb;
      this.attachWidgets();
      this.beforeComment();

      // Finally add suggestions text
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('',null,null));
      tr.appendChild(Cell('Comments and suggestions to M.Frank CERN/LHCb',2,'MonitorTinyHeader'));

      tb.appendChild(tr);
      tab.appendChild(tb);
      this.display.appendChild(tab);
      return this;
    };

    table.subscribe = function() {
      for(var i=0; i<this.subscriptions.length; ++i)
	this.subscriptions[i].subscribe(this.provider);
    };
    return table;
  };

  lhcb.widgets.SubdetectorPage.start = function(creator) {
    var msg  = the_displayObject['messages'];
    var body = document.getElementsByTagName('body')[0];
    var tips = init_tooltips(body);
    var display = creator(msg);

    body.appendChild(display);
    body.className = 'MainBody';
    if ( display.options.title ) setWindowTitle(display.options.title);
    if ( msg > 0 ) display.logger = new OutputLogger(display.logDisplay, 200, LOG_INFO, 'StatusLogger');
    else           display.logger = new OutputLogger(display.logDisplay,  -1, LOG_INFO, 'StatusLogger');
    display.provider = new DataProvider(display.logger);
    display.provider.topic = '/topic/status';
    display.build(display.options);
    display.subscribe();
    display.provider.start();
    body.style.cursor = 'default';
    return display;
  };

  lhcb.widgets.SubdetectorPage.stop = function(creator) {
    dataProviderReset();
  };

  ///=====================================================================================
  ///
  ///  VELO Widgets
  ///
  ///=====================================================================================
  lhcb.widgets.velo = new Object();

  /** VELO web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.velo.positionSummary = function(options) {
    var tb, tr, cell, tab = document.createElement('table');
    tab.className  = 'MonitorPage';
    tb = document.createElement('tbody');
    tooltips.set(tb,'Velo Position');
    tb.className  = 'MonitorPage';
    tb.height    = '120px';
    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('Velo Position',4,options.style));
      tb.appendChild(tr);
    }
    if ( options.legend ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('Position',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Opening',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Center',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Y:',1,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    if ( options.tips ) {
      tooltips.set(tb,options.tips);
    }
    if ( options.onclick ) {
      tb.onclick = options.onclick; 
    }
    tab.veloState   = StyledItem('lbWeb.LHCCOM/LHC.LHCb.Specific.VELO.Position', null, null);
    tab.veloOpening = StyledItem('lbWeb.LHCb_RunInfoCond.VELO.CurrentPosition.Opening',null,'%7.2f mm');
    tab.veloCenter  = StyledItem('lbWeb.LHCb_RunInfoCond.VELO.CurrentPosition.Center',null,'%7.2f mm');
    tab.veloY       = StyledItem('lbWeb.LHCb_RunInfoCond.VELO.CurrentPosition.Y',null,'%7.2f mm');
    tr = document.createElement('tr');
    tr.appendChild(tab.veloState);
    tr.appendChild(tab.veloOpening);
    tr.appendChild(tab.veloCenter);
    tr.appendChild(tab.veloY);
    tb.appendChild(tr);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.veloState);
      provider.subscribeItem(this.veloOpening);
      provider.subscribeItem(this.veloCenter);
      provider.subscribeItem(this.veloY);
    };
    return tab;
  };

  /** VELO web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.velo.moveSummary = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr = document.createElement('tr');
    var cell;
    tab.className = tb.className   = 'MonitorPage';
  
    tab.injectionAllowed = StyledItem('lbWeb.veloMove_RdBool.InjectionAllowed', null, null);
    tab.movableAllowed   = StyledItem('lbWeb.veloMove_RdBool.MovableDevicesAllowed', null, null);
    tab.safeBeam         = StyledItem('lbWeb.veloMove_RdBool.SafeStableBeam', null, null);
    if ( options.style ) {
      tr.appendChild(Cell('Velo Move Status',4,options.style));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    if ( options.legend ) {
      tr.appendChild(cell=Cell('Injection Allowed',1,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('Movable Devices Allowed',1,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('Safe Stable Beam',1,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    if ( options.tips ) {
      tooltips.set(tb,options.tips);
    }
    if ( options.onclick ) {
      tb.onclick = options.onclick; 
    }
    tr = document.createElement('tr');
    tr.appendChild(tab.injectionAllowed );
    tab.injectionAllowed.style.width = '33%';
    tr.appendChild(tab.movableAllowed);
    tab.movableAllowed.style.width = '33%';
    tr.appendChild(tab.safeBeam);
    tab.safeBeam.style.width = '33%';
    tb.appendChild(tr);
    tab.appendChild(tb);
  
    tab.subscribe = function(provider) {
      provider.subscribeItem(this.injectionAllowed);
      provider.subscribeItem(this.movableAllowed);
      provider.subscribeItem(this.safeBeam);      
    };
    return tab;
  };

  /** VELO web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.velo.vacuumSummary = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr = document.createElement('tr');

    tab.className = tb.className   = 'MonitorPage';
    tab.pe411 = StyledItem('lbWeb.veloVac_Meters.PE411.Vacuum', null, "%7.2e");
    tab.pe412 = StyledItem('lbWeb.veloVac_Meters.PE412.Vacuum', null, "%7.2e");
    tab.pe421 = StyledItem('lbWeb.veloVac_Meters.PE421.Vacuum', null, "%7.2e");
    tab.pe422 = StyledItem('lbWeb.veloVac_Meters.PE422.Vacuum', null, "%7.2e");
    if ( options.style ) {
      tr.appendChild(Cell('Velo Vacuum Status: Pressures in [hPa]',4,options.style));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    if ( options.legend ) {
      tr.appendChild(Cell('PE411',1,'MonitorDataHeader'));
      tr.appendChild(Cell('PE412',1,'MonitorDataHeader'));
      tr.appendChild(Cell('PE421',1,'MonitorDataHeader'));
      tr.appendChild(Cell('PE422',1,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    if ( options.tips ) {
      tooltips.set(tb,options.tips);
    }
    if ( options.onclick ) {
      tb.onclick = options.onclick; 
    }
    tr = document.createElement('tr');
    tr.appendChild(tab.pe411);
    tab.pe411.style.width = '25%';
    tr.appendChild(tab.pe412);
    tab.pe412.style.width = '25%';
    tr.appendChild(tab.pe421);
    tab.pe421.style.width = '25%';
    tr.appendChild(tab.pe422);
    tab.pe422.style.width = '25%';
    tb.appendChild(tr);
    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.pe411);
      provider.subscribeItem(this.pe412);
      provider.subscribeItem(this.pe421);
      provider.subscribeItem(this.pe422);
    };
    return tab;
  };

  /** VELO web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.velo.infrastructureSummary = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr = document.createElement('tr');
    var cell;

    tab.className = tb.className   = 'MonitorPage';
    tab.motion = FSMItem('lbWeb.VELO_MOTION',options.logger,true);
    tab.vacuum = FSMItem('lbWeb.VELO_VACUUM',options.logger,true);
    tab.cool   = FSMItem('lbWeb.VELO_COOLING',options.logger,true);
    tab.coolA  = FSMItem('lbWeb.VEA_COOL',options.logger,true);
    tab.coolC  = FSMItem('lbWeb.VEC_COOL',options.logger,true);

    if ( options.style ) {
      tr.appendChild(Cell('Velo Infrastructure State',3,options.style));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    if ( options.legend ) {
      tr.appendChild(Cell('Cooling',2,'MonitorDataHeader'));
      tr.appendChild(Cell('Side A',2,'MonitorDataHeader'));
      tr.appendChild(Cell('Side C',2,'MonitorDataHeader'));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    if ( options.tips ) {
      tooltips.set(tb,options.tips);
    }
    if ( options.onclick ) {
      tb.onclick = options.onclick; 
    }
    tr.appendChild(tab.cool);
    tr.appendChild(tab.cool.lock);
    tab.cool.style.width = '33%';
    tr.appendChild(tab.coolA);
    tr.appendChild(tab.coolA.lock);
    tab.coolA.style.width = '33%';
    tr.appendChild(tab.coolC);
    tr.appendChild(tab.coolC.lock);
    tab.coolC.style.width = '33%';
    tb.appendChild(tr);

    tr = document.createElement('tr');
    if ( options.legend ) {
      tr.appendChild(Cell('Vacuum',2,'MonitorDataHeader'));
      tr.appendChild(Cell('Motion',2,'MonitorDataHeader'));
      tr.appendChild(Cell('',2,null));
      tb.appendChild(tr);
      tr = document.createElement('tr');
    }
    tr.appendChild(tab.vacuum);
    tr.appendChild(tab.vacuum.lock);
    tab.vacuum.style.width = '30%';
    tr.appendChild(tab.motion);
    tr.appendChild(tab.motion.lock);
    tab.motion.style.width = '30%';
    tr.appendChild(cell=Cell('',2,null));
    cell.style.width='100%';
    tb.appendChild(tr);

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      provider.subscribeItem(this.motion);
      provider.subscribeItem(this.vacuum);
      provider.subscribeItem(this.cool);
      provider.subscribeItem(this.coolA);
      provider.subscribeItem(this.coolC);
    };
    return tab;
  };

  /** VELO web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.velo.LVBoards = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr, cell;

    tab.logger = options.logger;
    tab.className = tb.className   = 'MonitorPage';
    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('Velo LV Board State',3,options.style));
      tb.appendChild(tr);
    }
    if ( options.legend ) {
      tr = document.createElement('tr');
      tr.appendChild(cell=Cell('Board',1,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('SideA',2,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('SideC',2,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    tab.subscriptions=[];
    tab.addBoard = function(name) {
      var cell, tr = document.createElement('tr');
      tr.appendChild(cell=Cell(name,1,'MonitorDataHeader'));
      cell.style.width='30%';
      this['A_'+name] = FSMItem('lbWeb.VELOA_'+name,this.logger,true);
      this['C_'+name] = FSMItem('lbWeb.VELOC_'+name,this.logger,true);

      tr.appendChild(this['A_'+name]);
      tr.appendChild(this['A_'+name].lock);

      tr.appendChild(this['C_'+name]);
      tr.appendChild(this['C_'+name].lock);
      this.subscriptions.push(this['A_'+name]);
      this.subscriptions.push(this['C_'+name]);
      return tr;
    };

    tb.appendChild(tab.addBoard('DCS_LV_PS'));
    tb.appendChild(tab.addBoard('DCS_LV_EB'));
    tb.appendChild(tab.addBoard('DCS_LV_EB00'));
    tb.appendChild(tab.addBoard('DCS_LV_EB01'));
    tb.appendChild(tab.addBoard('DCS_LV_EB02'));
    tb.appendChild(tab.addBoard('DCS_LV_EB03'));
    tb.appendChild(tab.addBoard('DCS_LV_EB04'));
    tb.appendChild(tab.addBoard('DCS_LV_EB05'));
    tb.appendChild(tab.addBoard('DCS_LV_EB06'));
    tb.appendChild(tab.addBoard('DCS_LV_EB07'));
    tb.appendChild(tab.addBoard('DCS_LV_EB08'));
    tb.appendChild(tab.addBoard('DCS_LV_EB09'));

    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('Velo HV Board State',3,options.style));
      tb.appendChild(tr);
    }
    if ( options.legend ) {
      tr = document.createElement('tr');
      tr.appendChild(cell=Cell('Board',1,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('SideA',2,'MonitorDataHeader'));
      tr.appendChild(cell=Cell('SideC',2,'MonitorDataHeader'));
      tb.appendChild(tr);
    }
    tb.appendChild(tab.addBoard('HV_BOARD00'));
    tb.appendChild(tab.addBoard('HV_BOARD01'));
    tb.appendChild(tab.addBoard('HV_BOARD02'));
    tb.appendChild(tab.addBoard('HV_BOARD03'));
    tb.appendChild(tab.addBoard('HV_BOARD04'));
    tb.appendChild(tab.addBoard('HV_BOARD05'));

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      for(var i=0; i<this.subscriptions.length;++i) {
	provider.subscribeItem(this.subscriptions[i]);
      }
    };
    return tab;
  };

  ///=====================================================================================
  ///
  ///  DSS Widgets
  ///
  ///=====================================================================================
  lhcb.widgets.DSS = new Object();

  /** DSS web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.DSS.MixedWaterStatus = function(options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr, cell;

    tab.logger = options.logger;
    tab.subscriptions=[];
    tab.className = 'MonitorPage';
    tb.className   = 'MonitorPage';
    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell('DSS Mixed water temperature',6,options.style));
      tb.appendChild(tr);
    }
    tab.d1d2 = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_BARRACK_Mixed_Water_D1D2_In.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.d3A = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_BARRACK_Mixed_Water_D3A_In.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.d3B = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_BARRACK_Mixed_Water_D3B_In.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');

    tab.bunker = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_BUNKER_Mixed_Water_Cooling.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.caloA = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_CALO_Mixed_Water_Aside.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.caloC = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_CALO_Mixed_Water_Cside.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');

    tab.uxaA = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_UXA~A_Mixed_Water_Cooling.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.uxaB1 = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_UXA~B1_Mixed_Water_Cooling.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');
    tab.uxaC = StyledItem('lbWeb.dip/LHCb/DSS/PT_TE_UXA~C_Mixed_Water_Cooling.__DIP_DEFAULT__',null,'%7.2f <sup>o</sup>C');

    tab.subscriptions.push(tab.d1d2);
    tab.subscriptions.push(tab.d3A);
    tab.subscriptions.push(tab.d3B);
    tab.subscriptions.push(tab.bunker);
    tab.subscriptions.push(tab.caloA);
    tab.subscriptions.push(tab.caloC);
    tab.subscriptions.push(tab.uxaA);
    tab.subscriptions.push(tab.uxaB1);
    tab.subscriptions.push(tab.uxaC);
    
    tr = document.createElement('tr');
    tr.appendChild(Cell('D1/D2:',1,null));
    tr.appendChild(tab.d1d2);
    tr.appendChild(Cell('D3-A:',1,null));
    tr.appendChild(tab.d3A);
    tr.appendChild(Cell('D3-B:',1,null));
    tr.appendChild(tab.d3B);
    tb.appendChild(tr);

    tr = document.createElement('tr');
    tr.appendChild(Cell('Bunker:',1,null));
    tr.appendChild(tab.bunker);
    tr.appendChild(Cell('Calo-A:',1,null));
    tr.appendChild(tab.caloA);
    tr.appendChild(Cell('Calo-C:',1,null));
    tr.appendChild(tab.caloC);
    tb.appendChild(tr);

    tr = document.createElement('tr');
    tr.appendChild(Cell('UXA-A:',1,null));
    tr.appendChild(tab.uxaA);
    tr.appendChild(Cell('UXA-B1:',1,null));
    tr.appendChild(tab.uxaB1);
    tr.appendChild(Cell('UXA-C:',1,null));
    tr.appendChild(tab.uxaC);
    tb.appendChild(tr);

    tab.appendChild(tb);

    tab.subscribe = function(provider) {
      for(var i=0; i<this.subscriptions.length;++i) {
	provider.subscribeItem(this.subscriptions[i]);
      }
    };
    return tab;
  };

  /** DSS web widgets
   *
   *  @author  M.Frank
   */
  lhcb.widgets.DSS.CaVPlantStatus = function(sys, options) {
    var tab = document.createElement('table');
    var tb = document.createElement('tbody');
    var tr, cell;

    tab.subscriptions=[];
    tab.logger = options.logger;
    tab.className = tb.className = 'MonitorPage';
    if ( options.style ) {
      tr = document.createElement('tr');
      tr.appendChild(Cell(sys+' Cooling plant status',4,options.style));
      tb.appendChild(tr);
    }

    tab.OTspecific = function(tb) {
      this.outletPressure = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Parameters.param03',null,'%7.2f bar');
      this.outletTemperature = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Parameters.param04',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterIn  = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param13',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterOut = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param14',null,'%7.2f <sup>o</sup>C');
      this.subscriptions.push(this.outletPressure);
      this.subscriptions.push(this.outletTemperature);
      this.subscriptions.push(this.mixedWaterIn);
      this.subscriptions.push(this.mixedWaterOut);
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=Cell('Outlet',2,'MonitorDataHeader'));
      tr.appendChild(Cell('Press:',1,null));
      tr.appendChild(this.outletPressure);
      tr.appendChild(Cell('Temp:',1,null));
      tr.appendChild(this.outletTemperature);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Mixed water',2,'MonitorDataHeader'));
      tr.appendChild(Cell('In:',1,null));
      tr.appendChild(this.mixedWaterIn);
      tr.appendChild(Cell('Out:',1,null));
      tr.appendChild(this.mixedWaterOut);
    };

    tab.TTspecific = function(tb) {
      this.tt_manifold_temp01 = StyledItem('lbWeb.CaV/TtPlant.Actual.Measurements.param11',null,'%7.2f &#186;C');
      this.tt_manifold_temp02 = StyledItem('lbWeb.CaV/TtPlant.Actual.Measurements.param12',null,'%7.2f &#186;C');
      this.tt_manifold_temp03 = StyledItem('lbWeb.CaV/TtPlant.Actual.Measurements.param13',null,'%7.2f &#186;C');
      this.tt_manifold_temp04 = StyledItem('lbWeb.CaV/TtPlant.Actual.Measurements.param14',null,'%7.2f &#186;C');
      this.tt_c6f14flow       = StyledItem('lbWeb.CaV/TtPlant.Actual.Measurements.param18',null,'%7.2f l/h');
      this.mixedWaterIn  = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param16',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterOut = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param17',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterFlow = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param15',null,'%7.1f l/h');

      this.subscriptions.push(this.tt_manifold_temp01);
      this.subscriptions.push(this.tt_manifold_temp02);
      this.subscriptions.push(this.tt_manifold_temp03);
      this.subscriptions.push(this.tt_manifold_temp04);
      this.subscriptions.push(this.tt_c6f14flow);
      this.subscriptions.push(this.mixedWaterIn);
      this.subscriptions.push(this.mixedWaterOut);
      this.subscriptions.push(this.mixedWaterFlow);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=Cell('Manifold<br>Temperatures',1,'MonitorDataHeader'));
      cell.rowSpan = "2";
      tr.appendChild(Cell('Temp.1:',1,null));
      tr.appendChild(this.tt_manifold_temp01);
      tr.appendChild(Cell('Temp.2:',1,null));
      tr.appendChild(this.tt_manifold_temp02);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Temp.3:',1,null));
      tr.appendChild(this.tt_manifold_temp03);
      tr.appendChild(Cell('Temp.4:',1,null));
      tr.appendChild(this.tt_manifold_temp04);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Mixed water',1,'MonitorDataHeader'));
      tr.appendChild(Cell('In:',1,null));
      tr.appendChild(this.mixedWaterIn);
      tr.appendChild(Cell('Out:',1,null));
      tr.appendChild(this.mixedWaterOut);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Flow',1,'MonitorDataHeader'));
      tr.appendChild(Cell('C6F14:',1,null));
      tr.appendChild(this.tt_c6f14flow);
      tr.appendChild(Cell('Mixed:',1,null));
      tr.appendChild(this.mixedWaterFlow);
    };

    tab.ITspecific = function(tb) {
      this.it_manifold_temp01 = StyledItem('lbWeb.CaV/ItPlant.Actual.Measurements.param01',null,'%7.2f &#186;C');
      this.it_manifold_temp02 = StyledItem('lbWeb.CaV/ItPlant.Actual.Measurements.param02',null,'%7.2f &#186;C');
      this.it_c6f14flow = StyledItem('lbWeb.CaV/ItPlant.Actual.Measurements.param06',null,'%7.2f l/h');
      this.mixedWaterIn  = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param04',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterOut = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param05',null,'%7.2f <sup>o</sup>C');
      this.mixedWaterFlow = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.Measurements.param03',null,'%7.1f l/h');
      this.subscriptions.push(this.it_manifold_temp01);
      this.subscriptions.push(this.it_manifold_temp02);
      this.subscriptions.push(this.it_c6f14flow);
      this.subscriptions.push(this.mixedWaterIn);
      this.subscriptions.push(this.mixedWaterOut);
      this.subscriptions.push(this.mixedWaterFlow);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=Cell('Manifold',1,'MonitorDataHeader'));
      tr.appendChild(Cell('Temp.1:',1,null));
      tr.appendChild(this.it_manifold_temp01);
      tr.appendChild(Cell('Temp.2:',1,null));
      tr.appendChild(this.it_manifold_temp02);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Mixed water',1,'MonitorDataHeader'));
      tr.appendChild(Cell('In:',1,null));
      tr.appendChild(this.mixedWaterIn);
      tr.appendChild(Cell('Out:',1,null));
      tr.appendChild(this.mixedWaterOut);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(Cell('Flow',1,'MonitorDataHeader'));
      tr.appendChild(Cell('C6F14:',1,null));
      tr.appendChild(this.it_c6f14flow);
      tr.appendChild(Cell('Mixed:',1,null));
      tr.appendChild(this.mixedWaterFlow);
    };

    if      ( sys=='Ot'   ) tab.specific = tab.OTspecific;
    else if ( sys == 'It' ) tab.specific = tab.ITspecific;
    else if ( sys == 'Tt' ) tab.specific = tab.TTspecific;
    if ( tab.specific ) {
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=document.createElement('td'));
      cell.colSpan = 6;
      var t1, tb1;
      cell.appendChild(t1=document.createElement('table'));
      t1.appendChild(tb1=document.createElement('tbody'));
      t1.className = tb1.className = 'MonitorInnerFrame';
      tab.specific(tb1);
    }

    tab.fault    = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.fault',null,null);
    tab.alarm    = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.alarm',null,null);
    tab.fault.conversion = tab.alarm.conversion = function(data) 
    { return data=="FALSE" ? lhcb.widgets.green_smiley() : lhcb.widgets.red_smiley();  };
    tab.setLEDs = function(data) {
      if ( this.leds ) {
	for(var i=4; i<12; ++i) {
	  var m = data&(1<<i);
	  var l = this.leds[i-4];
	  l.style.color = data&(1<<i) ? 'black' : 'white';
	  l.bgColor     = data&(1<<i) ? '#00CC99' : '#333333';
	  l.innerHTML   = data&(1<<i) ? 'ON' : 'OFF';
	}
      }
    };
    tab.runState = StyledItem('lbWeb.CaV/'+sys+'Plant.Actual.status',null,null);
    tab.runState.parent = tab;
    tab.runState.conversion = function(data) {
      var t='';
      var v0 = parseInt(data);
      var v = v0&0xF;
      for(var i=0; i<4; ++i) {
	var m = v&(1<<i);
	if ( 0 != m ) {
	  if ( (v^m) != 0 ) {
	    this.className = 'FwDEAD';
	    this.parent.setLEDs(v0);
	    return 'Invalid ('+data+')';
	  }
	  else if ( i == 2 )  {
	    this.className = 'FwStateOKPhysics';
	    this.parent.setLEDs(v0);
	    return 'Run mode';
	  }
	  else if ( i == 0 )  {
	    this.className = 'FwStateAttention3';
	    this.parent.setLEDs(v0);
	    return 'System OFF';
	  }
	  else {
	    this.className = 'FwDEAD';
	    this.parent.setLEDs(v0);
	    if ( i == 0      ) return 'System OFF';
	    else if ( i == 1 ) return 'Stand-by mode';
	    else if ( i == 3 ) return 'Recovering';
	  }
	}
      }
      this.className = 'FwDEAD';
      return 'Invalid:'+data;
    };

    tab.subscriptions.push(tab.runState);

    tab.subscriptions.push(tab.fault);
    tab.subscriptions.push(tab.alarm);
    tb.appendChild(tr=document.createElement('tr'));
    tr.appendChild(Cell('State:',1,null));
    tr.appendChild(tab.runState);
    tr.appendChild(Cell('Alarms:',1,null));
    tr.appendChild(tab.alarm);
    tr.appendChild(Cell('Faults:',1,null));
    tr.appendChild(tab.fault);


    if ( options.showStatus ) {

      if ( options.style ) {
	tr = document.createElement('tr');
	tr.appendChild(Cell('Plant status bits:',4,options.style));
	tb.appendChild(tr);
      }
      tab.leds = [];
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=Cell('Leak flow rate:',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);
      tr.appendChild(cell=Cell('Leak search',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);

      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(cell=Cell('Safety heater:',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);
      tr.appendChild(cell=Cell('Reserved:',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);

      var r1, r2;
      tb.appendChild(tr=document.createElement('tr'));
      tr.appendChild(r1=cell=Cell('Maintenance:',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);
      tr.appendChild(r2=cell=Cell('TCR Control:',2,null));
      tr.appendChild(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell);

      // Reserved ones...
      tab.leds.push(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell=Cell('',1,'Text-Center'));
      tab.leds.push(cell=Cell('',1,'Text-Center'));
    }

    tab.appendChild(tb);
    tab.subscribe = function(provider) {
      for(var i=0; i<this.subscriptions.length;++i) {
	provider.subscribeItem(this.subscriptions[i]);
      }
    };
    return tab;

  };

  if ( _debugLoading ) alert('Script lhcb.display.widgets.cpp loaded successfully');
}

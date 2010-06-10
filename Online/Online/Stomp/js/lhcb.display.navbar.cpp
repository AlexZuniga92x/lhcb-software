_loadFile('lhcb.display.general','css');
var HTML_HEAD = new Object();

function setupHTML_HEAD() {
  HTML_HEAD.head = document.getElementsByTagName("head")[0];
  HTML_HEAD.url_base  = the_displayObject.url_base;
  HTML_HEAD.url_comet = lhcb.constants.lhcb_comet_url();
  HTML_HEAD.img_base  = lhcb.constants.lhcb_base_url();
  HTML_HEAD.img_base  = _fileBase;
};

function setupHTML_BASE(url) {
  var e = document.createElement("base");
  e.setAttribute("href",url);
  HTML_HEAD.head.appendChild(e);
};

var NavigationBar = function() {
  var table = document.createElement('table');
  table.body = document.createElement('tbody');
  table.haveSmallIcons = false;
  table.items = new Array();
  table.icons = null;
  table.className = 'NavigationBar';
  table.body.className = 'NavigationBar';
  table.style.width = '100%';
  table.body.style.width = '100%';
  table.cellSpacing = 2;
  table.addButton = function(text,tooltip,style, action) {
    var tr = document.createElement('tr');
    var td = document.createElement('td');
    td.style.verticalAlign = 'top';
    td.style.textAlign='center';
    td.className = style;
    td.innerHTML = text;
    td.onclick = action;
    td.client = this;
    tr.appendChild(td);
    this.body.appendChild(tr);
    return td;
  };
  
  table.open_abs_url = function(loc) {
    //alert(navigator.appName+'\n\n'+loc);
    doc = document;
    if ( parent.frames["viewer"] != null )  {
      if ( navigator.appName.indexOf("Netscape") != -1 )  {
	doc = parent.frames.viewer;
      }
      else if ( navigator.appName.indexOf("Microsoft") != -1 )  {
	doc = parent.viewer;
      }
      else {
	doc = parent.frames.viewer;
      }
    }
    doc.location = loc;
  };

  table.open_url = function(loc) {
    return this.open_abs_url(HTML_HEAD.url_comet+'/'+loc);
  };

  table.addSized = function(text, tooltip, url, style, small_pic, sx, sy, large_pic, lx, ly) {
    var item     = new Object();
    item.text    = text;
    item.url     = url;
    item.small   = small_pic;
    item.small_height = sy;
    item.small_width  = sx;

    item.large   = large_pic;
    item.large_height = ly;
    item.large_width  = lx;

    item.style        = style;
    item.img          = document.createElement('img');
    item.img.border   = 1;
    item.img.alt      = tooltip;
    //item.img.src      = item.large;
    //item.img.height   = item.large_height;
    //item.img.width    = item.large_width;
    this.items.push(item);
    return this;
  };

  table.add = function(text, tooltip, url, style, small_pic, large_pic) {
    return this.addSized(text, tooltip, url, style, small_pic, 16, 16, large_pic, 32, 32);
  };

  table.addURL = function(tooltip, url) {
    return this.addSized(url.title, tooltip, 
			 'JavaScript:navBar.open_abs_url("'+url.src+'")',
			 'NavigationBar',
			 url.img.icon, 16, 16, url.img.src, 32, 32);
  };

  table.setImages = function(small) {
    if ( small ) {
      if ( this.icons ) this.icons.innerHTML = 'Large Icons';
      for (var i=0, len=this.items.length; i < len; ++i)  {
	var item = this.items[i];
	item.img.src = item.small;
	item.img.height = item.small_height;
	item.img.width  = item.small_width;
      }
    }
    else {
      if ( this.icons ) this.icons.innerHTML = 'Small Icons';
      for (var i=0, len=this.items.length; i < len; ++i)  {
	var item = this.items[i];
	item.img.src = item.large;
	item.img.height = item.large_height;
	item.img.width  = item.large_width;
      }
    }
    return this;
  };

  table.build = function() {
    var item, itm, a, br, tr, td;
    for (var i=0, len=this.items.length; i < len; ++i)  {
      item = this.items[i];
      tr = document.createElement('tr');
      td = document.createElement('td');
      td.className = 'NavigationBar';
      td.style.verticalAlign = 'top';
      td.style.textAlign='center';
      td.style.border = 'solid';
      td.style.borderWidth = '1px';
      td.style.borderColor = '#000000';
      
      a = document.createElement('a');
      a.href = item.url;
      if ( item.style ) a.className = item.style;
      a.appendChild(item.img);
      a.appendChild(document.createElement('br'));
      a.appendChild(document.createTextNode(item.text));
      td.appendChild(a);
      tr.appendChild(td);
      this.body.appendChild(tr);
      item.cell = tr;
    }
    this.appendChild(this.body);
    return this.setImages(this.haveSmallIcons);
  };

  table.changeImages = function() {
    this.client.haveSmallIcons = !this.client.haveSmallIcons;
    return this.client.setImages(this.client.haveSmallIcons);
  };
  table.seeSubdetectors = function() 
  {   document.location = HTML_HEAD.url_base+'?type=navbar&system=subdetectors';  };
  table.seeGeneral = function() 
  {   document.location = HTML_HEAD.url_base+'?type=navbar';  };

  return table;
};


var navBar = null;

var navbar_general = function(navBar) {
  navBar.sdets = navBar.addButton('Subdetectors','See subdetector pages', 'DisplayButton', navBar.seeSubdetectors);

  //navBar.addURL('Got to the LHCb Online home page',lhcb.constants.lhcb.online_home_page);
  navBar.add('Help',
	     'Help',
	     'JavaScript:navBar.open_abs_url("News.htm")',
	     'NavigationBar',
	     '../Images/Help_16x16.gif',
	     '../Images/Help_32x32.gif');
  navBar.addURL('Show LHCb Page 1',lhcb.constants.urls.lhcb.page1);
  navBar.addURL('Show LHCb run status display',lhcb.constants.urls.lhcb.lhcb_run_status);
  navBar.addURL('Show FEST run status display',lhcb.constants.urls.lhcb.fest_run_status);
  navBar.addURL('Show subdetector run status display',lhcb.constants.urls.lhcb.sdet_run_status);
  navBar.addURL('Show collimator settings around Point 8',lhcb.constants.urls.lhcb.collimators);
  navBar.addURL('Show LHCb HV status',lhcb.constants.urls.lhcb.detstatus);
  navBar.addURL('Show Operations status',lhcb.constants.urls.lhcb.operstatus);
  lhcb.constants.urls.lhcb.bcm.src += '&charts=1';
  navBar.addURL('Show BCM status',lhcb.constants.urls.lhcb.bcm);
  navBar.addURL('Show Magnet status',lhcb.constants.urls.lhcb.magnet);
  navBar.addURL('LHC status around LHCb',lhcb.constants.urls.lhcb.lhc_status);
  navBar.addURL('LHCb electronic logbook',lhcb.constants.urls.lhcb.elog);
  /*
  navBar.addSized('Ramses',
		  'Radiation Monitoring in LHCb',
		  'JavaScript:navBar.open_url(lhcb.constants.lhcb_display_url("ramses"))',
		  'NavigationBar',
		  '../Images/LHCb/Ramses_icon.jpg',20,20,
		  '../Images/LHCb/Ramses_icon.jpg',32,32);
  */
};

var navbar_subdetectors = function(navBar) {
  navBar.sdets = navBar.addButton('LHCb general','Go back', 'DisplayButton', navBar.seeGeneral);

  navBar.add('Help',
	     'Help',
	     'JavaScript:navBar.open_abs_url("News.htm")',
	     'NavigationBar',
	     '../Images/Help_16x16.gif',
	     '../Images/Help_32x32.gif');
  navBar.addURL('Lumi/DAQ page',lhcb.constants.urls.lhcb.subdetectors.daq_lumi);
  navBar.addURL('Trigger/DAQ page',lhcb.constants.urls.lhcb.subdetectors.daq_trigger);
  navBar.addURL('Big Brother/DAQ page',lhcb.constants.urls.lhcb.subdetectors.daq_bigbrother);
  navBar.addURL('TRG/L0 status page',lhcb.constants.urls.lhcb.subdetectors.trg_l0status);
  navBar.addURL('Show VELO page',lhcb.constants.urls.lhcb.subdetectors.velo);
  navBar.addURL('Show ST page',lhcb.constants.urls.lhcb.subdetectors.st);
  navBar.addURL('Show OT page',lhcb.constants.urls.lhcb.subdetectors.ot);
  navBar.addURL('Show RICH page',lhcb.constants.urls.lhcb.subdetectors.rich);
  navBar.addURL('Show CALO page',lhcb.constants.urls.lhcb.subdetectors.calo);
  navBar.addURL('Show MUON page',lhcb.constants.urls.lhcb.subdetectors.muon);
  navBar.addURL('Show Cooling status page',lhcb.constants.urls.lhcb.subdetectors.cooling);
};


var navbar_unload = function()  {};

var navbar_body = function()  {
  setupHTML_HEAD();
  setupHTML_BASE(HTML_HEAD.url_base+'/..');
  var sys  = the_displayObject['system'];


  navBar = NavigationBar();
  navBar.icons = navBar.addButton('Small Icons','Change icon layout', 'DisplayButton', navBar.changeImages);

  if ( !sys ) {
    navbar_general(navBar);
  }
  else {
    navbar_subdetectors(navBar);
  }

  navBar.addSized('LHCb',
		  'LHCb home page',
		  'JavaScript:navBar.open_abs_url("'+lhcb.constants.urls.lhcb.home_page.src+'")',
		  'NavigationBar',
		  lhcb.constants.images.lhcb.icon,64,32,
		  lhcb.constants.images.lhcb.src,64,32);
  
  navBar.addURL(  'LHC collider home page',lhcb.constants.urls.lhc.project);
  navBar.addSized('Operations',
		  'CERN collider operations home page',
		  'JavaScript:navBar.open_abs_url("http://op-webtools.web.cern.ch/op-webtools/vistar/vistars.php?usr=SPS1")',
		  'NavigationBar',
		  lhcb.constants.images.beams_department.icon,80,16,
		  lhcb.constants.images.beams_department.src,120,32);
  navBar.addURL('CERN home page',lhcb.constants.urls.cern);
  
  navBar.build();
  HTML_HEAD.body = document.getElementsByTagName('body')[0];
  HTML_HEAD.body.appendChild(navBar);
  HTML_HEAD.body.className = 'NavigationBar';
  HTML_HEAD.body.style.width = '100%';
  HTML_HEAD.body.style.backgroundColor = '#666666';
  HTML_HEAD.body.style.color = '#FFFFFF';
  return navBar;
};

if ( _debugLoading ) alert('Script lhcb.display.navbar.cpp loaded successfully');

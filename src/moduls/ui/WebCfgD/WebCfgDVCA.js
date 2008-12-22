
//OpenSCADA system module UI.WebCfgD file: VCA.js
/***************************************************************************
 *   Copyright (C) 2008 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//> Global parameters init
MOD_ID = 'WebCfgD';	//Module identifier
stTmID = null;		//Status line timer identifier
pgRefrTmID = null;	//Periodic curent page update timer identifier
dlgWin = null;		//Opened window dialog
selPath = '';		//Selected node path
queSZ = 20;		//Previous and next arrays queue size
ndPrev = new Array();	//Previous nodes array
ndNext = new Array();	//Next nodes array
pgInfo = document.createElement('info');	//Curent page XML tree.
root = document.createElement('oscada_cntr');	//Root page's node.
selPath = '';		//Select item path
SEQ_XT = 0x01;		//Extended
SEQ_WR = 0x02;		//Write access
SEQ_RD = 0x04;		//Read access
copyBuf = '0';		//Copy node address buffer
//> Browser type detect
var isNN = navigator.appName.indexOf('Netscape') != -1;
var isIE = navigator.appName.indexOf('Microsoft') != -1;
var isOpera = navigator.appName.indexOf('Opera') != -1;
var isKonq = navigator.appName.indexOf('Konqueror') != -1;
/***************************************************
 * pathLev - Path parsing function.                *
 ***************************************************/
pathLev.off = 0;
function pathLev( path, level, scan )
{
  var an_dir = scan ? pathLev.off : 0;
  var t_lev = 0;
  var t_dir;
  while( an_dir < path.length && path.charAt(an_dir) == '/' ) an_dir++;
  if( an_dir >= path.length ) return '';
  while( true )
  {
    t_dir = path.indexOf('/',an_dir);
    if( t_dir < 0 ) { pathLev.off=path.length; return (t_lev==level)?path.substr(an_dir):''; }
    if( t_lev == level ) { pathLev.off=t_dir; return path.substr(an_dir,t_dir-an_dir); }
    an_dir = t_dir;
    t_lev++;
    while( an_dir<path.length && path.charAt(an_dir)=='/' ) an_dir++;
  }
}
/***************************************************
 * nodeText - Get DOM node text                    *
 ***************************************************/
function nodeText( node )
{
  var rez = '';
  if( node )
    for( var i = 0; i < node.childNodes.length; i++ )
      if( node.childNodes[i].nodeType == 3 ) rez += node.childNodes[i].data;
  return rez;
}
/***************************************************
 * setNodeText - Set DOM node text                 *
 ***************************************************/
function setNodeText( node, val )
{
  if( !node ) return;
  for( var i = 0; i < node.childNodes.length; i++ )
    if( node.childNodes[i].nodeType == 3 )
    { node.childNodes[i].data = val; return; }
  node.appendChild(node.ownerDocument.createTextNode(val));
}
/*****************************************************
 * nodeTextByTagId - Get DOM node by tag name and id *
 *****************************************************/
function nodeTextByTagId( node, tag, avl )
{
  for( var i = 0; i < node.childNodes.length; i++ )
    if( node.childNodes[i].nodeName == tag && node.childNodes[i].getAttribute('id') == avl )
      return nodeText(node.childNodes[i]);
  return null;
}
/***************************************************
 * posGetX - Get absolute position                 *
 ***************************************************/
function posGetX(obj,noWScrl)
{
  var posX = 0;
  for( ; obj != null; obj = obj.offsetParent ) posX += obj.offsetLeft;
  return posX+(!noWScrl?-window.pageXOffset:0)+(isNN?5:0);
}
/***************************************************
 * posGetY - Get absolute position                 *
 ***************************************************/
function posGetY(obj,noWScrl)
{
  var posY = 0;
  for( ; obj != null; obj = obj.offsetParent ) posY += obj.offsetTop;
  return posY+(!noWScrl?-window.pageYOffset:0)+(isNN?5:0);
}
/***************************************************
 * getXmlHttp - Check and return XMLHttpRequest for*
 *  various browsers.                              *
 ***************************************************/
function getXmlHttp( )
{
  if( window.XMLHttpRequest ) return new XMLHttpRequest();
  else if( window.ActiveXObject )
  {
    try { return new ActiveXObject('Msxml2.XMLHTTP'); }
    catch(e) { return new ActiveXObject('Microsoft.XMLHTTP'); }
  }
  return null;
}
/***************************************************
 * realRound - Real numbers round                  *
 ***************************************************/
function realRound( val, dig, toInt )
{
  rez = Math.floor(val*Math.pow(10,dig?dig:0)+0.5)/Math.pow(10,dig?dig:0);
  if( toInt ) return Math.floor(rez+0.5);
  return rez;
}
/***************************************************
 * servGet - XML get request to server             *
 ***************************************************/
function servGet( adr, prm )
{
  var req = getXmlHttp();
  req.open('GET',encodeURI('/'+MOD_ID+adr+'?'+prm),false);
  try
  { req.send(null);
    if( req.status == 200 && req.responseXML.childNodes.length )
      return req.responseXML.childNodes[0];
  } catch( e ) { window.location='/'; }
}
/***************************************************
 * servSet - XML set request to server             *
 ***************************************************/
function servSet( adr, prm, body, waitRez )
{
  var req = getXmlHttp();
  req.open('POST',encodeURI('/'+MOD_ID+adr+'?'+prm),!waitRez);
  try
  { req.send(body);
    if( waitRez && req.status == 200 && req.responseXML.childNodes.length )
      return req.responseXML.childNodes[0];
  } catch( e ) { window.location='/'; }
  return null;
}
/***************************************************
 * setStatus - Setup status message.               *
 ***************************************************/
function setStatus( mess, tm )
{
  setNodeText(document.getElementById('status'),mess?mess:'Ready');
  if( !mess ) return;
  if( stTmID ) clearTimeout(stTmID);
  if( !tm || tm > 0 ) stTmID = setTimeout('setStatus(null)',tm?tm:1000);
}
/***************************************************
 * expand - Expand/roll select subtree.            *
 ***************************************************/
function expand( el, val, upTree )
{
  if( !val )
  {
    for( var i = 0; i < el.childNodes.length; i++ )
      if( el.childNodes[i].nodeName == 'UL' )
      { el.removeChild(el.childNodes[i]); break; }
//    var treeRoot = document.getElementById('treeRoot');
//    if( treeRoot.scrollHeight <= treeRoot.offsetHeight ) treeRoot.childNodes[0].scrollIntoView();
  }
  else
  {
    var cUL = null;
    for( var i = 0; i < el.childNodes.length; i++ )
      if( el.childNodes[i].nodeName == 'UL' )
      { cUL = el.childNodes[i]; break; }
    if( !cUL )
    {
      cUL = document.createElement('ul');
      if( el.parentNode.lastChild!=el ) cUL.style.backgroundImage = 'url(img_treeLine)';
      el.appendChild(cUL);
    }
    if( el.grps.length > 1 )
    {
      //> Add and update present
      for( var i_g = 0; i_g < el.grps.length; i_g++ )
      {
	var liN = null;
	//> Search present item
	if( upTree )
	  for( var i_it = 0; i_it < cUL.childNodes.length; i_it++ )
	    if( cUL.childNodes[i_it].grps[0].getAttribute('id') == el.grps[i_g].getAttribute('id') )
	    { liN = cUL.childNodes[i_it]; break; }
	if( !liN )
	{
	  liN = document.createElement('li');
	  if( i_g >= cUL.childNodes.length ) cUL.appendChild(liN);
	  else cUL.insertBefore(liN,cUL.childNodes[i_g]);
	  liN.isExpand = false;
	}
	liN.setAttribute('id',el.getAttribute('id'));
	//> Set group
	liN.grps = new Array(); liN.grps.push(el.grps[i_g]);
	//> Init links
	var isUsable = parseInt(liN.grps[0].getAttribute('chPresent'));
	var treeIco = '/'+MOD_ID+'/img_tree'+(isUsable?(liN.isExpand?'Minus':'Plus'):'')+'Up'+((i_g!=(el.grps.length-1))?'Down':'');
	var liCont = isUsable?"<a class='pm' onclick='expand(this.parentNode,!this.parentNode.isExpand); return false;'>":"";
	liCont += "<img src='"+treeIco+"'/></a>";
	liCont += "<span style='font-style: italic;'>"+el.grps[i_g].getAttribute('dscr')+":</span>";
	//>> Next node for update
	if( upTree && liN.isExpand )
	{
	  var liNUL = null;
	  for( var i = 0; i < liN.childNodes.length; i++ ) if( liN.childNodes[i].nodeName == 'UL' ) { liNUL = liN.childNodes[i]; break; }
	  liN.innerHTML = liCont;
	  if( liNUL ) liN.appendChild(liNUL);
	  expand(liN,val,upTree);
	}
	else liN.innerHTML = liCont;
      }
      //> Delete no present
      if( upTree )
	for( var i_it = 0; i_it < cUL.childNodes.length; i_it++ )
	{
	  var i_g;
	  for( i_g = 0; i_g < el.grps.length; i_g++ )
	    if( cUL.childNodes[i_it].grps[0].getAttribute('id') == el.grps[i_g].getAttribute('id') )
	      break;
	  if( i_g >= el.grps.length ) { cUL.removeChild(cUL.childNodes[i_it]); i_it--; }
	}
    }
    else
    {
      var grpId = el.grps[0].getAttribute('id');
      var hostN = servGet(el.getAttribute('id'),'com=chlds&grp='+grpId);
      if( hostN && parseInt(hostN.getAttribute('rez'))==0 && hostN.childNodes.length )
      {
	//> Add and update present
	for( var i_e = 0; i_e < hostN.childNodes.length; i_e++ )
	{
	  var tmpNdId = el.getAttribute('id')+'/'+(grpId+(hostN.childNodes[i_e].getAttribute('id')?hostN.childNodes[i_e].getAttribute('id'):nodeText(hostN.childNodes[i_e])));
	  var liN = null;
	  //>> Find item
	  if( upTree )
	    for( var i_it = 0; i_it < cUL.childNodes.length; i_it++ )
	      if( cUL.childNodes[i_it].getAttribute('id') == tmpNdId )
	      { liN = cUL.childNodes[i_it]; break; }
	  if( !liN )
	  {
	    liN = document.createElement('li');
	    if( i_e >= cUL.childNodes.length ) cUL.appendChild(liN);
	    else cUL.insertBefore(liN,cUL.childNodes[i_e]);
	    liN.isExpand = false;
	  }
	  liN.setAttribute('id',tmpNdId);
	  //> Load groups
	  liN.grps = new Array();
	  for( var i_grp = 0; i_grp < hostN.childNodes[i_e].childNodes.length; i_grp++ )
	    if( hostN.childNodes[i_e].childNodes[i_grp].nodeName == 'grp' )
	      liN.grps.push(hostN.childNodes[i_e].childNodes[i_grp]);
	  //> Init links
	  var isUsable = (liN.grps.length>1)||(liN.grps.length&&parseInt(liN.grps[0].getAttribute('chPresent')));
	  var treeIco = '/'+MOD_ID+'/img_tree'+(isUsable?(liN.isExpand?'Minus':'Plus'):'')+'Up'+((i_e!=(hostN.childNodes.length-1))?'Down':'');
	  var liCont = isUsable?"<a class='pm' onclick='expand(this.parentNode,!this.parentNode.isExpand); return false;'>":"";
	  liCont += "<img src='"+treeIco+"'/></a>";
	  if( parseInt(hostN.childNodes[i_e].getAttribute('icoSize')) )
	    liCont += "<span><img height='16px' src='/"+MOD_ID+liN.getAttribute('id')+"?com=ico'/></span>";
	  liCont += "<span><a onclick='selectPage(this.parentNode.parentNode.getAttribute(\"id\")); return false;' "+
	    "onmouseover='setStatus(this.parentNode.parentNode.getAttribute(\"id\"),10000);' href='#'>"+nodeText(hostN.childNodes[i_e])+"</a></span>";
	  //>> Next node for update
	  if( upTree && liN.isExpand )
	  {
	    var liNUL = null;
	    for( var i_eu = 0; i_eu < liN.childNodes.length; i_eu++ ) if( liN.childNodes[i_eu].nodeName == 'UL' ) { liNUL = liN.childNodes[i_eu]; break; }
	    liN.innerHTML = liCont;
	    if( liNUL ) liN.appendChild(liNUL);
	    expand(liN,val,upTree);
	  }
	  else liN.innerHTML = liCont;
	}
	//> Delete no present
	if( upTree )
	  for( var i_it = 0; i_it < cUL.childNodes.length; i_it++ )
	  {
	    var i_e;
	    for( i_e = 0; i_e < hostN.childNodes.length; i_e++ )
	    {
	      var grpId = el.grps[0].getAttribute('id');
	      var tmpNdId = el.getAttribute('id')+'/'+(grpId+(hostN.childNodes[i_e].getAttribute('id')?hostN.childNodes[i_e].getAttribute('id'):nodeText(hostN.childNodes[i_e])));
	      if( cUL.childNodes[i_it].getAttribute('id') == tmpNdId ) break;
	    }
	    if( i_e >= hostN.childNodes.length ) { cUL.removeChild(cUL.childNodes[i_it]); i_it--; }
	  }
      }
    }
  }

  if( el.isExpand != val )
  {
    //> Change tree icon
    el.childNodes[0].childNodes[0].src = val ? el.childNodes[0].childNodes[0].src.replace('Plus','Minus') :
					       el.childNodes[0].childNodes[0].src.replace('Minus','Plus');
    el.isExpand = val;
  }
}
/***************************************************
 * selectPage - Select node by user.               *
 ***************************************************/
function selectPage( path )
{
  if( !path ) return;

  //> Prev and next
  if( selPath.length ) ndPrev.push(selPath);
  while( ndPrev.length >= queSZ ) ndPrev.shift();
  ndNext = new Array();

  //> Page display
  pageDisplay(path);
}
/***************************************************
 * pageDisplay - Display selected page.            *
 ***************************************************/
function pageDisplay( path )
{
  if( !path ) return;

  //> Chek Up
  actEnable('actUp',path.lastIndexOf('/') != -1 && path.lastIndexOf('/') != 0);

  //> Check Prev and Next
  actEnable('actPrevious',ndPrev.length);
  actEnable('actNext',ndNext.length);

  if( path != pgInfo.getAttribute('path') )
  {
    //> Stop refresh
    pageCyclRefrStop();

    if( selPath.length && document.getElementById(selPath) ) document.getElementById(selPath).className = '';
    selPath = path;
    if( selPath.length && document.getElementById(selPath) ) document.getElementById(selPath).className = 'select';
    setNodeText(document.getElementById('selPath'),selPath);

    pgInfo = servGet(selPath,'com=info');
    if( parseInt(pgInfo.getAttribute('rez'))!=0 ) { alert(nodeText(pgInfo)); return; }
    pgInfo.setAttribute('path',selPath);
    root = pgInfo.childNodes[0];
  }
  else
  {
    //>> Check the new node structure and the old node
    var iTree = servGet(selPath,'com=info');
    if( parseInt(iTree.getAttribute('rez'))!=0 ) { alert(nodeText(iTree)); return; }
    if( chkStruct(root,iTree.childNodes[0]) )
    { pgInfo = iTree; pgInfo.setAttribute('path',selPath); root = pgInfo.childNodes[0]; }
  }

  selectChildRecArea(root,'/',null);

  //> The add and the delete access allow check
  actEnable('actAddIt',false);
  for( var i_ch = 0; i_ch < root.childNodes.length; i_ch++ )
    if( root.childNodes[i_ch].getAttribute('id') == 'br' )
    {
      for( var i_g = 0; i_g < root.childNodes[i_ch].childNodes.length; i_g++ )
        if( parseInt(root.childNodes[i_ch].childNodes[i_g].getAttribute('acs'))&SEQ_WR )
        { actEnable('actAddIt',true); break; }
      break;
    }
  actEnable('actDelIt',parseInt(root.getAttribute('acs'))&SEQ_WR)

  //> Load and Save allow check
  var reqModif = servGet(selPath,'com=modify');
  actEnable('actLoad',parseInt(nodeText(reqModif)));
  actEnable('actSave',parseInt(nodeText(reqModif)));

  //> Edit tools update
  editToolUpdate( );
}
/***************************************************
 * editToolUpdate - Edit action state update.      *
 ***************************************************/
function editToolUpdate( )
{
  actEnable('actCut',(selPath.length&&parseInt(root.getAttribute('acs'))&SEQ_WR));
  actEnable('actCopy',selPath.length);
  actEnable('actPaste',false);

  //> Src and destination elements calc
  if( copyBuf.length <= 1 || copyBuf.substr(1) == selPath || pathLev(copyBuf.substr(1),0) != pathLev(selPath,0) ) return;
  var s_elp; var s_el; var t_el;
  pathLev.off = 0;
  while( (t_el=pathLev(copyBuf.substr(1),0,true)).length )
  { s_elp += ('/'+s_el); s_el = t_el; }

  for( var i_ch = 0; i_ch < root.childNodes.length; i_ch++ )
    if( root.childNodes[i_ch].getAttribute('id') == 'br' )
    {
      for( var i_g = 0; i_g < root.childNodes[i_ch].childNodes.length; i_g++ )
        if( parseInt(root.childNodes[i_ch].childNodes[i_g].getAttribute('acs'))&SEQ_WR )
        { actEnable('actPaste',true); break; }
      break;
    }
  if( parseInt(root.getAttribute('acs'))&SEQ_WR ) actEnable('actPaste',true);
}
/***************************************************
 * selectChildRecArea - Make page content          *
 ***************************************************/
function selectChildRecArea( node, aPath, cBlk )
{
  var i_area = 0;
  //> View title name
  if( aPath == "/" )
  {
    //>> Set node icon
    document.getElementById('pgIco').src = (nodeTextByTagId(node,'img','ico') != null) ? ('/'+MOD_ID+selPath+'?com=ico') : '';
    //>> Set title
    setNodeText(document.getElementById('pgTitle'),node.getAttribute('dscr'));
    //>> Delete tabs of deleted areas
    var activeTab = null;
    var tabs = document.getElementById('pgTabs');
    for( var i_tbs = 0; i_tbs < tabs.childNodes.length; i_tbs++ )
    {
      var i_cf;
      for( i_cf = 0; i_cf < node.childNodes.length; i_cf++ )
	if( node.childNodes[i_cf].nodeName.toLowerCase() == 'area' && nodeText(tabs.childNodes[i_tbs]) == node.childNodes[i_cf].getAttribute('dscr') )
	  break;
      if( i_cf >= node.childNodes.length )
      {
	if( tabs.childNodes[i_tbs].className == 'active' ) node.childNodes[i_tbs].setAttribute('qview','0');
	tabs.removeChild(tabs.childNodes[i_tbs]);
	i_tbs--;
      }
      else if( tabs.childNodes[i_tbs].className == 'active' ) activeTab = tabs.childNodes[i_tbs];
    }
    //>> Add new tabs
    for( var i_cf = 0; i_cf < node.childNodes.length; i_cf++ )
    {
      if( node.childNodes[i_cf].nodeName.toLowerCase() != 'area' ) continue;
      var i_tbs;
      for( i_tbs = 0; i_tbs < tabs.childNodes.length; i_tbs++ )
	if( nodeText(tabs.childNodes[i_tbs]) == node.childNodes[i_cf].getAttribute('dscr') )
	  break;
      if( i_tbs >= tabs.childNodes.length )
      {
	 var itab = document.createElement('span');
	 setNodeText(itab,node.childNodes[i_cf].getAttribute('dscr'));
	 itab.onclick = function( ) { tabSelect(this); return false; }
	 if( i_area >= tabs.childNodes.length ) tabs.appendChild(itab);
	 else tabs.insertBefore(itab,tabs.childNodes[i_area]);
	 node.childNodes[i_cf].setAttribute('qview','0');
      }
      i_area++;
    }
    if( !activeTab && tabs.childNodes.length ) { activeTab = tabs.childNodes[0]; activeTab.className = 'active'; }
    //>> Prepare active tab
    for( var i_cf = 0; i_cf < node.childNodes.length; i_cf++ )
      if( node.childNodes[i_cf].nodeName.toLowerCase() == 'area' && nodeText(activeTab) == node.childNodes[i_cf].getAttribute('dscr') )
      {
        var refresh = parseInt(node.childNodes[i_cf].getAttribute('qview'));
        var cPg = document.getElementById('pgCont');
	if( !refresh )
	{
	  while( cPg.childNodes.length ) cPg.removeChild(cPg.childNodes[0]);
	  selectChildRecArea(node.childNodes[i_cf],aPath+node.childNodes[i_cf].getAttribute('id')+'/',cPg);
	  //>>> Mark last drawed tabs
	  node.childNodes[i_cf].setAttribute('qview','1');
	}
	else selectChildRecArea(node.childNodes[i_cf],aPath+node.childNodes[i_cf].getAttribute('id')+'/',null);
      }
    return;
  }
  else for( var i_cf = 0; i_cf < node.childNodes.length; i_cf++ )
  {
    var t_s = node.childNodes[i_cf];

    //>> Check access to node
    var wr = parseInt(t_s.getAttribute('acs'))&SEQ_WR;

    //>> View areas
    if( t_s.nodeName.toLowerCase() == 'area' )
    {
      var cntBlk = cBlk;
      if( cntBlk )
      {
        var cntBlk = document.createElement('fieldset');
        cntBlk.className = 'elem';
        cntBlk.appendChild(document.createElement('legend'));
        cntBlk.childNodes[0].appendChild(document.createTextNode(t_s.getAttribute('dscr')));
        cBlk.appendChild(cntBlk);
      }
      selectChildRecArea(t_s,aPath+t_s.getAttribute('id')+'/',cntBlk);
    }
    //>> View list elements
    else if( t_s.nodeName.toLowerCase() == 'list' )
    {
      var brPath = (aPath+t_s.getAttribute('id')).replace(/%/g,'%25').replace(/\//g,'%2f');
      var lab = null; var val = null;

      if( cBlk )
      {
	var dBlk = document.createElement('div'); dBlk.className = 'elem';
	lab = document.createElement('span'); lab.className = 'label';
	dBlk.appendChild(lab);
	dBlk.appendChild(document.createElement('br'));
	val = document.createElement('select'); val.className = 'list';
	val.size = 10;
	val.srcNode = t_s;
	val.itPath = selPath+'/'+brPath;
	val.onmouseover = function() { setStatus(this.itPath,10000); }
	val.onclick = function(e)
	{
	  if( !e ) e = window.event;
	  var popUpMenu = getPopup();
	  while( popUpMenu.childNodes[0].childNodes.length ) popUpMenu.childNodes[0].removeChild(popUpMenu.childNodes[0].childNodes[0]);
	  var optEl = null;
	  if( this.srcNode.getAttribute('tp') == 'br' && this.selectedIndex >= 0 )
	  {
	    optEl = document.createElement('option'); setNodeText(optEl,'Go'); optEl.posId = 'go'; popUpMenu.childNodes[0].appendChild(optEl);
	    optEl = document.createElement('option'); setNodeText(optEl,'----------'); optEl.disabled = true; popUpMenu.childNodes[0].appendChild(optEl);
	  }
	  if( (parseInt(this.srcNode.getAttribute('acs'))&SEQ_WR) && this.srcNode.getAttribute('s_com') )
	  {
	    if( this.srcNode.getAttribute('s_com').search('add') != -1 )
	    { optEl = document.createElement('option'); setNodeText(optEl,'Add'); optEl.posId = 'add'; popUpMenu.childNodes[0].appendChild(optEl); }
	    if( this.srcNode.getAttribute('s_com').search('ins') != -1 && this.selectedIndex >= 0 )
	    { optEl = document.createElement('option'); setNodeText(optEl,'Insert'); optEl.posId = 'ins'; popUpMenu.childNodes[0].appendChild(optEl); }
	    if( this.srcNode.getAttribute('s_com').search('edit') != -1 && this.selectedIndex >= 0 )
	    { optEl = document.createElement('option'); setNodeText(optEl,'Edit'); optEl.posId = 'edit'; popUpMenu.childNodes[0].appendChild(optEl); }
	    if( this.srcNode.getAttribute('s_com').search('del') != -1 && this.selectedIndex >= 0 )
	    { optEl = document.createElement('option'); setNodeText(optEl,'Delete'); optEl.posId = 'del'; popUpMenu.childNodes[0].appendChild(optEl); }
	    if( this.srcNode.getAttribute('s_com').search('move') != -1 && this.selectedIndex >= 0 )
	    {
	      optEl = document.createElement('option'); setNodeText(optEl,'----------'); optEl.disabled = true; popUpMenu.childNodes[0].appendChild(optEl);
	      optEl = document.createElement('option'); setNodeText(optEl,'Up'); optEl.posId = 'up'; popUpMenu.childNodes[0].appendChild(optEl);
	      optEl = document.createElement('option'); setNodeText(optEl,'Down'); optEl.posId = 'down'; popUpMenu.childNodes[0].appendChild(optEl);
	    }
	  }
	  if( popUpMenu.childNodes[0].childNodes.length )
	  {
	    popUpMenu.srcNode = this.srcNode;
	    popUpMenu.itPath = this.itPath;
	    if( this.selectedIndex >= 0 )
	    {
	      popUpMenu.lsId = this.options[this.selectedIndex].lsId;
	      popUpMenu.lsText = this.options[this.selectedIndex].value;
	      if( !popUpMenu.lsId ) popUpMenu.lsId = popUpMenu.lsText;
	    }
	    popUpMenu.childNodes[0].size = Math.max(3,popUpMenu.childNodes[0].childNodes.length);
	    popUpMenu.style.cssText = 'visibility: visible; left: '+e.clientX+'px; top: '+e.clientY+'px;';
	    popUpMenu.childNodes[0].focus();
	    popUpMenu.childNodes[0].selectedIndex = -1;
	    popUpMenu.childNodes[0].onclick = function()
	    {
	      this.parentNode.style.cssText = 'visibility: hidden; left: -200px; top: -200px;';
	      if( this.selectedIndex < 0 || !this.options[this.selectedIndex].posId ) return;
	      var idm = parseInt(this.parentNode.srcNode.getAttribute('idm'));
	      var posId = this.options[this.selectedIndex].posId;
	      if( posId == 'go' )
	        selectPage(selPath+'/'+(this.parentNode.srcNode.getAttribute('br_pref')+this.parentNode.lsId).replace(/%/g,'%25').replace(/\//g,'%2f'));
	      else if( posId == 'add' || posId == 'ins' || posId == 'edit' )
	      {
		dlgWin = ReqIdNameDlg('/'+MOD_ID+'/ico');
		var fIdVal = dlgWin.document.getElementById('id').childNodes[1].childNodes[0];
		fIdVal.maxLength = this.parentNode.srcNode.getAttribute('idSz');
		if( !fIdVal.maxLength ) fIdVal.maxLength = 1000;
		dlgWin.document.getElementById('type').style.display = 'none';
		dlgWin.document.getElementById('name').style.display = idm?'':'none';
		var actOkFld = dlgWin.document.getElementById('actOk');
		actOkFld.itPath = this.parentNode.itPath;
		actOkFld.srcNode = this.parentNode.srcNode;
		actOkFld.selectedIndex = this.selectedIndex;
		actOkFld.lsId = this.parentNode.lsId;
		actOkFld.lsText = this.parentNode.lsText;
		if( posId == 'add' )
		{
		  setNodeText(dlgWin.document.getElementById('title').childNodes[1],'Add new element.');
		  actOkFld.onclick = function()
		  {
		    var idm = dlgWin.document.getElementById('name').style.display!='none';
		    var inpId = dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value;
		    var inpName = idm ? dlgWin.document.getElementById('name').childNodes[1].childNodes[0].value : inpId;
		    var rez = servSet(this.itPath,'com=com',"<add "+(idm?("id='"+inpId+"'"):"")+">"+inpName+"</add>",true);
		    if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		    if( this.srcNode.getAttribute('tp') == 'br' ) treeUpdate();
		    pageRefresh(); dlgWin.close();
		    return false;
		  }
		}
		else if( posId == 'ins' )
		{
		  setNodeText(dlgWin.document.getElementById('title').childNodes[1],'Insert new element.');
		  actOkFld.onclick = function()
		  {
		    var idm = dlgWin.document.getElementById('name').style.display!='none';
		    var inpId = dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value;
		    var inpName = idm ? dlgWin.document.getElementById('name').childNodes[1].childNodes[0].value : inpId;
		    var com = "<ins "+(idm?("id='"+inpId+"' "):"")+" pos='"+this.selectedIndex+"' p_id='"+this.lsId+"'>"+inpName+"</ins>";
		    var rez = servSet(this.itPath,'com=com',com,true);
		    if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		    if( this.srcNode.getAttribute('tp') == 'br' ) treeUpdate();
		    pageRefresh();
		    dlgWin.close(); return false;
		  }
		}
		else if( posId == 'edit' )
		{
		  setNodeText(dlgWin.document.getElementById('title').childNodes[1],'Rename element.');
		  dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value = idm ? this.parentNode.lsId : this.parentNode.lsText;
		  if( idm ) dlgWin.document.getElementById('name').childNodes[1].childNodes[0].value = this.parentNode.lsText;
		  actOkFld.onclick = function()
		  {
		    var idm = dlgWin.document.getElementById('name').style.display!='none';
		    var inpId = dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value;
		    var inpName = idm ? dlgWin.document.getElementById('name').childNodes[1].childNodes[0].value : inpId;
		    var com = "<edit "+(idm?("id='"+inpId+"' "):"")+" pos='"+this.selectedIndex+"' p_id='"+this.lsId+"'>"+inpName+"</edit>";
		    var rez = servSet(this.itPath,'com=com',com,true);
		    if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		    if( this.srcNode.getAttribute('tp') == 'br' ) treeUpdate();
		    pageRefresh();
		    dlgWin.close(); return false;
		  }
		}
	      }
	      else if( posId == 'up' || posId == 'down' )
	      {
		var c_new = (posId == 'down') ? c_new = this.selectedIndex+1 : this.selectedIndex-1;
		var rez = servSet(this.parentNode.itPath,'com=com',"<move pos='"+this.selectedIndex+"' to='"+c_new+"'/>",true);
		if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		pageRefresh();
	      }
	      else if( posId == 'del' )
	      {
		var com = idm ? ("<del pos='"+this.selectedIndex+"' id='"+this.parentNode.lsId+"'/>") :
				("<del pos='"+this.selectedIndex+"'>"+this.parentNode.lsText+"</del>");
		var rez = servSet(this.parentNode.itPath,'com=com',com,true);
		if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		if( this.parentNode.srcNode.getAttribute('tp') == 'br' ) treeUpdate();
		pageRefresh();
	      }
	      return false;
	    }
	  }
	  return false;
	}
	dBlk.appendChild(val);
	cBlk.appendChild(dBlk);

	t_s.addr_lab = lab; t_s.addr_val = val;
      }
      else { lab = t_s.addr_lab; val = t_s.addr_val; }
      //>>> Fill list
      setNodeText(lab,t_s.getAttribute('dscr')+': ');
      val.title = t_s.getAttribute('help');

      var dataReq = servGet(selPath+'/'+brPath,'com=get');
      if( !dataReq || parseInt(dataReq.getAttribute('rez'))!=0 ) { alert(nodeText(dataReq)); continue; }

      while( val.childNodes.length ) val.removeChild(val.childNodes[0]);
      for( var i_el = 0; i_el < dataReq.childNodes.length; i_el++ )
	if( dataReq.childNodes[i_el].nodeName.toLowerCase() == 'el' )
	{
	  var opt = document.createElement('option');
	  opt.lsId = dataReq.childNodes[i_el].getAttribute('id');
	  setNodeText(opt,nodeText(dataReq.childNodes[i_el]));
	  val.appendChild(opt);
	}
      while( val.childNodes.length < 4 ) { var opt = document.createElement('option'); opt.disabled = true; val.appendChild(opt); }
      val.size = Math.min(10,Math.max(4,val.childNodes.length));
    }
    //>> View images
    else if( t_s.nodeName.toLowerCase() == "img" )
    {
      var brPath = (aPath+t_s.getAttribute('id')).replace(/%/g,'%25').replace(/\//g,'%2f').replace(/%/g,'%25');
      var lab = null; var val = null;

      if( cBlk )
      {
	var dBlk = document.createElement('div'); dBlk.className = 'elem';
	lab = document.createElement('span'); lab.className = 'label';
	dBlk.appendChild(lab);
	if( !t_s.getAttribute("v_sz") || parseInt(t_s.getAttribute("v_sz")) > 70 ) dBlk.appendChild(document.createElement('br'));
	val = document.createElement('img'); val.className = 'picture';
	if( t_s.getAttribute('h_sz') ) val.style.width = t_s.getAttribute('h_sz')+'px';
	if( t_s.getAttribute("v_sz") ) val.style.height = t_s.getAttribute("v_sz")+'px';
	val.itPath = selPath+'/'+brPath;
	val.onmouseover = function() { setStatus(this.itPath,10000); }
	if( wr )
	{
	  val.style.cursor = 'pointer';
	  val.onclick = function( )
	  {
	    dlgWin = ReqIdNameDlg('/'+MOD_ID+'/img_save','Select image file for download to picture field.','/'+MOD_ID+this.itPath+'?com=img');
	    dlgWin.document.getElementById('type').style.display = 'none';
	    dlgWin.document.getElementById('id').style.display = 'none';
	    var nmFld = dlgWin.document.getElementById('name');
	    nmFld.style.display = '';
	    nmFld.childNodes[1].childNodes[0].type = 'file';
	    dlgWin.document.getElementById('actOk').type = 'submit';
	    if( !isKonq ) dlgWin.document.getElementById('formBlk').onsubmit = function( ) { setTimeout('dlgWin.close(); pageRefresh();',200); }
	    else dlgWin.document.getElementById('actCancel').onclick = function( ) { setTimeout('dlgWin.close(); pageRefresh();',200); }
	    return false;
	  }
	}
	dBlk.appendChild(val);
	cBlk.appendChild(dBlk);

	t_s.addr_lab = lab; t_s.addr_val = val;
      }
      else { lab = t_s.addr_lab; val = t_s.addr_val; }
      //>>> Set image
      if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
      if( val )
      {
        val.title = t_s.getAttribute('help');
        val.src = '/'+MOD_ID+selPath+'/'+brPath+'?com=img&rnd='+Math.floor(Math.random()*1000);
      }
    }
    //>> View standart fields
    else if( t_s.nodeName.toLowerCase() == 'fld' ) basicFields(t_s,aPath,cBlk,wr);
  }
}
/***************************************************
 * basicFields - Prepare basic fields view         *
 ***************************************************/
function basicFields( t_s, aPath, cBlk, wr, comm )
{
  var brPath = (aPath+t_s.getAttribute('id')).replace(/%/g,'%25').replace(/\//g,'%2f');

  var dataReq = document.createElement('get');
  if( !comm )
  {
    dataReq = servGet(selPath+'/'+brPath,'com=get');
    if( !dataReq || parseInt(dataReq.getAttribute('rez'))!=0 ) { alert(nodeText(dataReq)); setNodeText(dataReq,''); }
  }

  //> View select fields
  if( t_s.getAttribute('dest') == 'select' )
  {
    var lab = null; var val_r = null; var val_w = null;
    if( cBlk )
    {
      //>> View info
      if( !wr )
      {
	val_r = document.createElement('span'); val_r.className = 'const';
	val_r.StatusTip = selPath+'/'+brPath; val_r.onmouseover = function() { setStatus(this.StatusTip,10000); }
      }
      //>> View edit
      else
      {
	val_w = document.createElement('span');	val_w.className = 'line';
	val_w.appendChild(document.createElement('select'));
	val_w.childNodes[0].itPath = selPath+'/'+brPath;
	val_w.childNodes[0].onchange = function( )
	{
	  if( this.selectedIndex < 0 ) return;
	  var selId = this.options[this.selectedIndex].getAttribute('vid');
	  var selVal = nodeText(this.options[this.selectedIndex]);
	  var rez = servSet(this.itPath,'com=com','<set>'+(selId?selId:selVal)+'</set>',true);
	  if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	  setTimeout('pageRefresh()',500);
	  return false;
	}
	val_w.StatusTip = selPath+'/'+brPath; val_w.onmouseover = function() { setStatus(this.StatusTip,10000); }
      }
      //>> Check use label
      if( t_s.getAttribute('dscr') )
      {
	lab = document.createElement('span'); lab.className = 'label';
	cBlk.dBlk = document.createElement('div'); cBlk.dBlk.className = 'elem'; cBlk.dBlk.appendChild(lab);
	if( val_w ) cBlk.dBlk.appendChild(val_w);
	if( val_r ) cBlk.dBlk.appendChild(val_r);
	cBlk.appendChild(cBlk.dBlk);
      }
      else
      {
	if( val_w ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_w); else { delete val_w; val_w = null; } }
	if( val_r ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_r); else { delete val_r; val_r = null; } }
      }
      t_s.addr_lab = lab; t_s.addr_val_r = val_r; t_s.addr_val_w = val_w;
    }
    else { lab = t_s.addr_lab; val_r = t_s.addr_val_r; val_w = t_s.addr_val_w; }
    //>> Fill combo
    if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
    if( val_w || val_r )
    {
      (val_r||val_w).title = t_s.getAttribute('help');
      var sel_ok = false, c_el = 0;
      if( !t_s.getAttribute('select') )
      {
	var ind_ls = t_s.getAttribute('sel_id') ? t_s.getAttribute('sel_id').split(';') : new Array();
	var val_ls = t_s.getAttribute('sel_list').split(';');
	var valWCfg = '';
	for( var ls_i = 0; ls_i < val_ls.length; ls_i++ )
	{
	  if(val_w) valWCfg+="<option "+
		(ind_ls.length ? ("vid='"+ind_ls[ls_i]+"' "+((ind_ls[ls_i]==nodeText(dataReq))?"selected='true'":""))
			       : ((val_ls[ls_i]==nodeText(dataReq))?"selected='true'":""))+">"+val_ls[ls_i]+"</option>";
	  if( (ind_ls.length && ind_ls[ls_i] == nodeText(dataReq) ) || (!ind_ls.length && val_ls[ls_i] == nodeText(dataReq)) )
	  { sel_ok = true; if(val_r) setNodeText(val_r,val_ls[ls_i]); }
	}
      }
      else
      {
	var x_lst = servGet(selPath+'/'+(t_s.getAttribute('select').replace(/%/g,'%25').replace(/\//g,'%2f')),'com=get');
	if( x_lst )
	  for( var i_el = 0; i_el < x_lst.childNodes.length; i_el++ )
	  {
	    if( x_lst.childNodes[i_el].nodeName.toLowerCase() != 'el' ) continue;
	    var curElId = x_lst.childNodes[i_el].getAttribute('id');
	    var curElVl = nodeText(x_lst.childNodes[i_el]);
	    if( val_w ) valWCfg+="<option "+
		(curElId ? ("vid='"+curElId+"' "+((curElId==nodeText(dataReq))?"selected='true'":""))
			 : ((curElVl==nodeText(dataReq))?"selected='true'":""))+">"+curElVl+"</option>";
	    if( (curElId && curElId == nodeText(dataReq)) || (!curElId && curElVl == nodeText(dataReq)) )
	    { sel_ok = true; if(val_r) setNodeText(val_r,curElVl); }
	  }
      }
      ///>>> Insert empty field if none selected
      if( !sel_ok )
      {
	if(val_w) valWCfg+="<option selected='true'>"+nodeText(dataReq)+"</option>";
	if(val_r) setNodeText(val_r,nodeText(dataReq));
      }
      if( val_w ) val_w.childNodes[0].innerHTML = valWCfg;
    }
  }
  else
  {
    //> View boolean fields
    if( t_s.getAttribute('tp') == 'bool' )
    {
      var lab = null; var val_r = null; var val_w = null;
      if( cBlk )
      {
	//>> View info
	if( !wr )
	{
	  val_r = document.createElement('span'); val_r.className = 'const';
	  val_r.StatusTip = selPath+'/'+brPath; val_r.onmouseover = function() { setStatus(this.StatusTip,10000); }
	}
	//>> View edit
	else
	{
	  val_w = document.createElement('span'); val_w.className = 'line';
	  val_w.appendChild(document.createElement('input'));
	  val_w.childNodes[0].type = 'checkbox'; val_w.childNodes[0].itPath = selPath+'/'+brPath;
	  val_w.childNodes[0].onclick = function( )
	  {
	    var rez = servSet(this.itPath,'com=com','<set>'+(this.checked?'1':'0')+'</set>',true);
	    if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	    setTimeout('pageRefresh()',500);
	    return false;
	  }
	  val_w.StatusTip = selPath+'/'+brPath; val_w.onmouseover = function() { setStatus(this.StatusTip,10000); }
	}
	//>> Check use label
	if( t_s.getAttribute('dscr') )
	{
	  lab = document.createElement('span'); lab.className = 'label';
	  cBlk.dBlk = document.createElement('div'); cBlk.dBlk.className = 'elem'; cBlk.dBlk.appendChild(lab);
	  if( val_w ) cBlk.dBlk.appendChild(val_w);
	  if( val_r ) cBlk.dBlk.appendChild(val_r);
	  cBlk.appendChild(cBlk.dBlk);
	}
	else
	{
	  if( val_w ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_w); else { delete val_w; val_w = null; } }
	  if( val_r ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_r); else { delete val_r; val_r = null; } }
	}
	t_s.addr_lab = lab; t_s.addr_val_r = val_r; t_s.addr_val_w = val_w;
      }
      else { lab = t_s.addr_lab; val_r = t_s.addr_val_r; val_w = t_s.addr_val_w; }
      //>> Fill CheckBox
      if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
      if( val_w )
      {
	val_w.title = t_s.getAttribute('help');
	val_w.childNodes[0].checked = parseInt(nodeText(dataReq));
      }
      if( val_r )
      {
	val_r.title = t_s.getAttribute('help');
	setNodeText(val_r,parseInt(nodeText(dataReq))?'On':'Off');
      }
    }
    //> View edit fields
    else if( t_s.getAttribute('tp') == 'str' && (t_s.getAttribute('rows') || t_s.getAttribute('cols')) )
    {
      var lab = null; var edit = null;
      if( cBlk )
      {
	dBlk = document.createElement('div'); dBlk.className = 'elem';
	lab = document.createElement('span'); lab.className = 'label';
	edit = document.createElement('textarea');
	edit.itPath = selPath+'/'+brPath;
	edit.readOnly = !wr;
	if( t_s.getAttribute('cols') ) edit.setAttribute('cols',parseInt(t_s.getAttribute('cols')));
	else edit.setAttribute('wrap','off');
	edit.setAttribute('rows',parseInt(t_s.getAttribute('rows')) ? parseInt(t_s.getAttribute('rows')):5);
	edit.onkeyup = function()
	{
	  if( !this.isChanged && this.value != this.defaultValue )
	  {
	    var btBlk = document.createElement('div'); btBlk.style.textAlign = 'right';
	    var btApply = document.createElement('input'); btApply.type = 'button'; btApply.value = 'Apply';
	    btApply.onclick = function()
	    {
	      var wEl = this.parentNode.parentNode;
	      wEl.childNodes[2].value;
	      var rez = servSet(wEl.childNodes[2].itPath,'com=com','<set>'+wEl.childNodes[2].value+'</set>',true);
	      if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	      setTimeout('pageRefresh()',500);
	      wEl.removeChild(this.parentNode);
	      wEl.childNodes[2].isChanged = false;
	      return false;
	    }
	    var btCancel = document.createElement('input'); btCancel.type = 'button'; btCancel.value = 'Cancel';
	    btCancel.onclick = function()
	    {
	      var wEl = this.parentNode.parentNode;
	      wEl.childNodes[2].value = wEl.childNodes[2].defaultValue;
	      wEl.removeChild(this.parentNode);
	      wEl.childNodes[2].isChanged = false;
	      return false;
	    }
	    btBlk.appendChild(btApply); btBlk.appendChild(btCancel); this.parentNode.appendChild(btBlk);
	    this.isChanged = true;
	  }
	  else if( this.isChanged && this.value == this.defaultValue && this.parentNode.childNodes[3] )
	  { this.parentNode.removeChild(this.parentNode.childNodes[3]); this.isChanged = false; }
	  return true;
	}
	edit.StatusTip = selPath+'/'+brPath;
	edit.onmouseover = function() { setStatus(this.StatusTip,10000); }
	dBlk.appendChild(lab); dBlk.appendChild(document.createElement('br')); dBlk.appendChild(edit); cBlk.appendChild(dBlk);
	t_s.addr_lab = lab; t_s.addr_edit = edit;
      }
      else { lab = t_s.addr_lab; edit = t_s.addr_edit; }
      //>> Fill Edit
      if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
      if( edit && !edit.isChanged )
      {
	edit.title = t_s.getAttribute('help');
	edit.value = edit.defaultValue = nodeText(dataReq);
      }
    }
    //> View Data-Time fields
    else if( t_s.getAttribute('tp') == 'time' )
    {
      var lab = null; var val_r = null; var val_w = null;
      if( cBlk )
      {
	//>> View info
	if( !wr )
	{
	  val_r = document.createElement('span'); val_r.className = 'const';
	  val_r.StatusTip = selPath+'/'+brPath; val_r.onmouseover = function() { setStatus(this.StatusTip,10000); }
	}
	//>> View edit
	else
	{
	  val_w = document.createElement('span'); val_w.className = 'line number';
	  val_w.itPath = selPath+'/'+brPath;
	  val_w.innerHTML = "<input type='text' size='2'/><input type='text' size='2'/><input type='text' size='4'/>&nbsp;"+
		"<input type='text' size='2'/><input type='text' size='2'/><input type='text' size='2'/>"
	  val_w.childNodes[0].onkeyup = val_w.childNodes[1].onkeyup = val_w.childNodes[2].onkeyup =
		val_w.childNodes[4].onkeyup = val_w.childNodes[5].onkeyup = val_w.childNodes[6].onkeyup = function(e)
	  {
	    if( !e ) e = window.event;
	    if( this.parentNode.isEdited && e.keyCode == 13 ) { this.parentNode.childNodes[7].onclick(); return true; }
	    if( this.parentNode.isEdited && e.keyCode == 27 )
	    {
	      var val_w = this.parentNode;
	      for( var i_ch = 0; i_ch < val_w.childNodes.length; i_ch++ )
	        if( val_w.childNodes[i_ch].defaultValue )
	          val_w.childNodes[i_ch].value = val_w.childNodes[i_ch].defaultValue;
	      val_w.removeChild(this.parentNode.childNodes[7]);
	      val_w.isEdited = false;
	      return true;
	    }
	    if( this.parentNode.isEdited || this.value == this.defaultValue ) return true;
	    var btOk = document.createElement('img'); btOk.src = '/'+MOD_ID+'/img_button_ok';
	    btOk.onclick = function( )
	    {
	      var val_w = this.parentNode;
	      var dt = new Date(0);
	      dt.setDate(parseInt(val_w.childNodes[0].value)); dt.setMonth(parseInt(val_w.childNodes[1].value)-1); dt.setFullYear(parseInt(val_w.childNodes[2].value));
	      dt.setHours(parseInt(val_w.childNodes[4].value)); dt.setMinutes(parseInt(val_w.childNodes[5].value)); dt.setSeconds(parseInt(val_w.childNodes[6].value));
	      var rez = servSet(val_w.itPath,'com=com','<set>'+Math.floor(dt.getTime()/1000)+'</set>',true);
	      if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	      setTimeout('pageRefresh()',500);
	      val_w.removeChild(this);
	      val_w.isEdited = false;
	      return false;
	    }
	    this.parentNode.appendChild(btOk);
	    this.parentNode.isEdited = true;
	    return true;
	  }
	  val_w.StatusTip = selPath+'/'+brPath; val_w.onmouseover = function() { setStatus(this.StatusTip,10000); }
	}
	//>> Check use label
	if( t_s.getAttribute('dscr') )
	{
	  lab = document.createElement('span'); lab.className = 'label';
	  cBlk.dBlk = document.createElement('div'); cBlk.dBlk.className = 'elem'; cBlk.dBlk.appendChild(lab);
	  if( val_w ) cBlk.dBlk.appendChild(val_w);
	  if( val_r ) cBlk.dBlk.appendChild(val_r);
	  cBlk.appendChild(cBlk.dBlk);
	}
	else
	{
	  if( val_w ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_w); else { delete val_w; val_w = null; } }
	  if( val_r ) { if( cBlk.dBlk ) cBlk.dBlk.appendChild(val_r); else { delete val_r; val_r = null; } }
	}
	t_s.addr_lab = lab; t_s.addr_val_r = val_r; t_s.addr_val_w = val_w;
      }
      else { lab = t_s.addr_lab; val_r = t_s.addr_val_r; val_w = t_s.addr_val_w; }
      //>> Fill data
      if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
      if( val_w && !val_w.isEdited )
      {
	val_w.title = t_s.getAttribute('help');
	var dt = new Date(parseInt(nodeText(dataReq))*1000);
	val_w.childNodes[0].value = val_w.childNodes[0].defaultValue = dt.getDate();
	val_w.childNodes[1].value = val_w.childNodes[1].defaultValue = dt.getMonth()+1;
	val_w.childNodes[2].value = val_w.childNodes[2].defaultValue = dt.getFullYear();
	val_w.childNodes[4].value = val_w.childNodes[4].defaultValue = dt.getHours();
	val_w.childNodes[5].value = val_w.childNodes[5].defaultValue = dt.getMinutes();
	val_w.childNodes[6].value = val_w.childNodes[6].defaultValue = dt.getSeconds();
      }
      if( val_r )
      {
	val_r.title = t_s.getAttribute('help');
	var dt = new Date(parseInt(nodeText(dataReq))*1000);
	setNodeText(val_r,dt.getDate()+'.'+(dt.getMonth()+1)+'.'+dt.getFullYear()+' '+dt.getHours()+':'+dt.getMinutes()+':'+dt.getSeconds());
      }
    }
    //> View other string and numberic fields
    else
    {
      var lab = null; var val_r = null; var val_w = null;
      if( cBlk )
      {
	//>> View info
	if( !wr )
	{
	  val_r = document.createElement('span'); val_r.className = 'const';
	  val_r.StatusTip = selPath+'/'+brPath; val_r.onmouseover = function() { setStatus(this.StatusTip,10000); }
	}
	//>> View edit
	else
	{
	  val_w = document.createElement('span'); val_w.className = 'line';
	  val_w.itPath = selPath+'/'+brPath;
	  val_w.appendChild(document.createElement('input')); val_w.childNodes[0].type = 'text';
	  val_w.StatusTip = selPath+'/'+brPath; val_w.onmouseover = function() { setStatus(this.StatusTip,10000); }

	  var tp = t_s.getAttribute('tp');
	  if( t_s.getAttribute('dest') == 'sel_ed' )
	  {
	    val_w.childNodes[0].size = 20;
	    var cmbImg = document.createElement('img'); cmbImg.src = '/'+MOD_ID+'/img_combar';
	    cmbImg.onclick = function( )
	    {
	      if( !this.parentNode.sel_list || !this.parentNode.sel_list.length ) return false;
	      var combMenu = getCombo();
	      var optHTML = '';
	      for( var i_l = 0; i_l < this.parentNode.sel_list.length; i_l++ )
		optHTML += '<option>'+this.parentNode.sel_list[i_l]+'</option>';
	      var edFld = this.parentNode.childNodes[0];
	      combMenu.childNodes[0].edFld = edFld;
	      combMenu.childNodes[0].innerHTML = optHTML;
	      combMenu.childNodes[0].size = Math.max(3,this.parentNode.sel_list.length);
	      combMenu.style.cssText = 'visibility: visible; left: '+posGetX(edFld,true)+'px; '+
				       'top: '+(posGetY(edFld,true)+edFld.offsetHeight)+'px; '+
				       'width: '+edFld.offsetWidth+'px';
	      combMenu.childNodes[0].focus();
	      combMenu.childNodes[0].onclick = function()
	      {
		this.parentNode.style.cssText = 'visibility: hidden; left: -200px; top: -200px;';
		if( this.selectedIndex < 0 ) return;
		this.edFld.value = nodeText(this.options[this.selectedIndex]);
		var rez = servSet(this.edFld.parentNode.itPath,'com=com','<set>'+this.edFld.value+'</set>',true);
		if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
		setTimeout('pageRefresh()',500);
		return false;
	      }
	      return false;
	    }
	    val_w.appendChild(cmbImg);
	  }
	  else if( tp == 'dec' )
	  {
	    val_w.className += ' number';
	    val_w.childNodes[0].size = 7;
	    var spinImg = document.createElement('img'); spinImg.src = '/'+MOD_ID+'/img_spinar';
	    spinImg.onclick = function(e)
	    {
	      if(!e) e = window.event;
	      var val_w = this.parentNode.childNodes[0];
	      val_w.value = parseInt(val_w.value)+(((e.clientY-posGetY(this))<10)?1:-1);
	      val_w.onkeyup();
	      return false;
	    }
	    val_w.appendChild(spinImg);
	  }
	  else if( tp == 'hex' || tp == 'oct' || tp == 'real' ) { val_w.className += ' number'; val_w.childNodes[0].size = 7; }
	  else
	  {
	    val_w.childNodes[0].size = 30;
	    val_w.childNodes[0].maxLength = t_s.getAttribute('len');
	    if( !val_w.childNodes[0].maxLength ) val_w.childNodes[0].maxLength = 1000;
	  }

	  val_w.childNodes[0].onkeyup = function(e)
	  {
	    if( !e ) e = window.event;
	    if( this.parentNode.isEdited && e.keyCode == 13 ) { this.parentNode.lastChild.onclick(); return true; }
	    if( this.parentNode.isEdited && e.keyCode == 27 )
	    {
	      this.value = this.defaultValue;
	      this.parentNode.isEdited = false;
	      this.parentNode.removeChild(this.parentNode.lastChild);
	      return true;
	    }
	    if( this.parentNode.isEdited || this.value == this.defaultValue ) return true;
	    var btOk = document.createElement('img'); btOk.src = '/'+MOD_ID+'/img_button_ok';
	    btOk.onclick = function( )
	    {
	      var rez = servSet(this.parentNode.itPath,'com=com','<set>'+this.parentNode.childNodes[0].value+'</set>',true);
	      if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	      setTimeout('pageRefresh()',500);
	      this.parentNode.isEdited = false;
	      this.parentNode.removeChild(this);
	      return false;
	    }
	    this.parentNode.appendChild(btOk);
	    this.parentNode.isEdited = true;
	    return true;
	  }
	}
	//>> Check use label
	if( t_s.getAttribute('dscr') )
	{
	  lab = document.createElement('span'); lab.className = 'label';
	  cBlk.dBlk = document.createElement('div'); cBlk.dBlk.className = 'elem'; cBlk.dBlk.appendChild(lab);
	  if( val_w ) cBlk.dBlk.appendChild(val_w);
	  if( val_r ) cBlk.dBlk.appendChild(val_r);
	  cBlk.appendChild(cBlk.dBlk);
	}
	else
	{
	  if( val_w ) { if(cBlk.dBlk) cBlk.dBlk.appendChild(val_w); else { delete val_w; val_w = null; } }
	  if( val_r ) { if(cBlk.dBlk) cBlk.dBlk.appendChild(val_r); else { delete val_r; val_r = null; } }
	}
	t_s.addr_lab = lab; t_s.addr_val_r = val_r; t_s.addr_val_w = val_w;
      }
      else { lab = t_s.addr_lab; val_r = t_s.addr_val_r; val_w = t_s.addr_val_w; }
      //>> Fill line
      var sval = nodeText(dataReq);
      if( t_s.getAttribute('tp') == 'hex' ) sval = '0x'+parseInt(nodeText(dataReq)).toString(16);
      else if( t_s.getAttribute('tp') == 'oct' ) sval = '0'+parseInt(nodeText(dataReq)).toString(8);

      if( lab ) setNodeText(lab,t_s.getAttribute('dscr')+':');
      if( val_r )
      {
	val_r.title = t_s.getAttribute('help');
	setNodeText(val_r,sval);
      }
      if( val_w && !val_w.isEdited )
      {
	val_w.title = t_s.getAttribute('help');
	val_w.childNodes[0].value = val_w.childNodes[0].defaultValue = sval;
	//>> Load combo list
	if( t_s.getAttribute('dest') == 'sel_ed' )
	{
	  if( !t_s.getAttribute('select') ) val_w.sel_list = t_s.getAttribute('sel_list').split(';');
	  else
	  {
	    val_w.sel_list = new Array();
	    var x_lst = servGet(selPath+'/'+(t_s.getAttribute('select').replace(/%/g,'%25').replace(/\//g,'%2f')),'com=get');
	    if( x_lst )
	      for( var i_el = 0; i_el < x_lst.childNodes.length; i_el++ )
		if( x_lst.childNodes[i_el].nodeName.toLowerCase() == 'el' )
		  val_w.sel_list.push(nodeText(x_lst.childNodes[i_el]));
	  }
	}
      }
    }
  }
}
/***************************************************
 * actEnable - Actions enable.                     *
 ***************************************************/
function actEnable( act, vl )
{
  var actEl = document.getElementById(act);
  if( !actEl ) return;
  actEl.className = vl?'active':'inactive';
  actEl.childNodes[0].src = vl ? actEl.childNodes[0].src.replace('filtr=gray','filtr=none') :
				 actEl.childNodes[0].src.replace('filtr=none','filtr=gray');
}
/***************************************************
 * chkStruct - Info page tree check structure.     *
 ***************************************************/
function chkStruct( w_nd, n_nd )
{
  //> Check access
  if( w_nd.getAttribute('acs') != n_nd.getAttribute('acs') ) return true;

  //> Scan deleted nodes
  for( var i_w = 0; i_w < w_nd.childNodes.length; i_w++ )
  {
    var i_n;
    for( i_n = 0; i_n < n_nd.childNodes.length; i_n++ )
      if( w_nd.childNodes[i_w].nodeName.toLowerCase() == n_nd.childNodes[i_n].nodeName.toLowerCase() && 
	    w_nd.childNodes[i_w].getAttribute('id') == n_nd.childNodes[i_n].getAttribute('id') )
	break;
    if( i_n >= n_nd.childNodes.length ) return true;
  }

  //> Scan for new nodes and check present nodes
  for( var i_n = 0; i_n < n_nd.childNodes.length; i_n++ )
  {
    var i_w;
    for( i_w = 0; i_w < w_nd.childNodes.length; i_w++ )
      if( w_nd.childNodes[i_w].nodeName.toLowerCase() == n_nd.childNodes[i_n].nodeName.toLowerCase() &&
	    w_nd.childNodes[i_w].getAttribute('id') == n_nd.childNodes[i_n].getAttribute('id') )
	break;
    if( i_w >= w_nd.childNodes.length ) return true;
    else if( chkStruct(w_nd.childNodes[i_w],n_nd.childNodes[i_n]) ) return true;

    //> Check of the description present
    if( (w_nd.childNodes[i_w].getAttribute('dscr') && !n_nd.childNodes[i_n].getAttribute('dscr')) ||
	(!w_nd.childNodes[i_w].getAttribute('dscr') && n_nd.childNodes[i_n].getAttribute('dscr')) )
      return true;

    //> Check base fields destination change
    if( w_nd.childNodes[i_w].nodeName.toLowerCase() == 'fld' &&
	(w_nd.childNodes[i_w].getAttribute('dest') != n_nd.childNodes[i_n].getAttribute('dest') ||
	w_nd.childNodes[i_w].getAttribute('tp') != n_nd.childNodes[i_n].getAttribute('tp')) )
      return true;
  }

  return false;
}
/***************************************************
 * tabSelect - Select page tab                     *
 ***************************************************/
function tabSelect( tab )
{
  if( !tab || tab.className == 'active' ) return;
  for( var i_t = 0; i_t < tab.parentNode.childNodes.length; i_t++ )
    if( tab.parentNode.childNodes[i_t].className == 'active' )
      tab.parentNode.childNodes[i_t].className = '';
  tab.className = 'active';

  for( var i_cf = 0; i_cf < root.childNodes.length; i_cf++ )
    if( root.childNodes[i_cf].nodeName.toLowerCase() == 'area' && nodeText(tab) == root.childNodes[i_cf].getAttribute('dscr') )
    { root.childNodes[i_cf].setAttribute('qview','0'); break; }

  pageCyclRefrStop();
  pageDisplay(selPath);
}
/***************************************************
 * hostsUpdate - Init/update tree hosts.           *
 ***************************************************/
function hostsUpdate( )
{
  var treeRoot = document.getElementById('treeRoot');
  //> Make hosts
  var hostN = servGet('/','com=chlds');
  if( hostN )
  {
    //> Remove no present hosts
    for( var i_top = 0; i_top < treeRoot.childNodes.length; i_top++ )
    {
      var i_h;
      for( i_h = 0; i_h < hostN.childNodes.length; i_h++ )
        if( treeRoot.childNodes[i_top].getAttribute('id') == ('/'+hostN.childNodes[i_h].getAttribute('id')) )
          break;
      if( i_h >= hostN.childNodes.length ) { treeRoot.removeChild(treeRoot.childNodes[i_top]); i_top--; }
    }
    //> Add/update hosts
    var emptyTree = treeRoot.childNodes.length;
    for( var i = 0; i < hostN.childNodes.length; i++ )
    {
      var liN = null;
      if( !emptyTree )
        for( var i_top = 0; i_top < treeRoot.childNodes.length; i_top++ )
        {
//          alert(treeRoot.childNodes[i_top]);
          if( treeRoot.childNodes[i_top].getAttribute('id') == ('/'+hostN.childNodes[i].getAttribute('id')) )
          { liN = treeRoot.childNodes[i_top]; break; }
        }
      if( !liN ) { liN = document.createElement('li'); treeRoot.appendChild(liN); liN.isExpand = false; }
      liN.setAttribute('id','/'+hostN.childNodes[i].getAttribute('id'));
      //> Load groups
      liN.grps = new Array();
      for( var i_grp = 0; i_grp < hostN.childNodes[i].childNodes.length; i_grp++ )
	if( hostN.childNodes[i].childNodes[i_grp].nodeName.toLowerCase() == 'grp' )
	  liN.grps.push(hostN.childNodes[i].childNodes[i_grp]);
      //> Init links
      var treeIco = '/'+MOD_ID+'/img_tree'+(liN.isExpand?'Minus':'Plus')+((i!=0)?'Up':'')+((i!=(hostN.childNodes.length-1))?'Down':'');
      var liCont = "<a class='pm' onclick='expand(this.parentNode,!this.parentNode.isExpand); return false;'><img src='"+treeIco+"'/></a>";
      if( parseInt(hostN.childNodes[i].getAttribute('icoSize')) )
	liCont += "<span><img height='16px' src='/"+MOD_ID+liN.getAttribute('id')+"?com=ico'/></span>";
      liCont += "<span><a onclick='selectPage(this.parentNode.parentNode.getAttribute(\"id\")); return false;' "+
	"onmouseover='setStatus(this.parentNode.parentNode.getAttribute(\"id\"),10000);' href='#'>"+nodeText(hostN.childNodes[i])+"</a></span>";
      liN.innerHTML = liCont;
    }
    if( hostN.childNodes.length ) return '/'+hostN.childNodes[0].getAttribute('id');
  }
}
/**************************************************
 * treeUpdate - Update tree expanded elements.    *
 **************************************************/
function treeUpdate( )
{
  var treeRoot = document.getElementById('treeRoot');
  for( var i_t = 0; i_t < treeRoot.childNodes.length; i_t++ )
    if( treeRoot.childNodes[i_t].isExpand )
      expand(treeRoot.childNodes[i_t],true,true);
}
/**************************************************
 * getPopup - Get popup menu.                     *
 **************************************************/
function getPopup( )
{
  var popUpMenu = document.getElementById('popupmenu');
  if( !popUpMenu )
  {
    popUpMenu = document.createElement('div'); popUpMenu.id = 'popupmenu';
    popUpMenu.appendChild(document.createElement('select'));
    popUpMenu.childNodes[0].onblur = function() { this.parentNode.style.visibility = 'hidden'; }
    document.body.appendChild(popUpMenu);
    popUpMenu.style.visibility = 'hidden';
  }
  return popUpMenu;
}
/**************************************************
 * getCombo - Get combo menu.                     *
 **************************************************/
function getCombo( )
{
  var comboMenu = document.getElementById('combomenu');
  if( !comboMenu )
  {
    comboMenu = document.createElement('div'); comboMenu.id = 'combomenu';
    comboMenu.appendChild(document.createElement('select'));
    comboMenu.childNodes[0].onblur = function() { this.parentNode.style.visibility = 'hidden'; }
    document.body.appendChild(comboMenu);
    comboMenu.style.visibility = 'hidden';
  }
  return comboMenu;
}
/**************************************************
 * pageRefresh - Curent page refrash call.        *
 **************************************************/
function pageRefresh( )
{
  pageDisplay(selPath);
  if( pgRefrTmID ) pgRefrTmID = setTimeout('pageRefresh();',5000);
}
/********************************************************
 * pageCyclRefrStart - Start curent page cyclic refrash.*
 ********************************************************/
function pageCyclRefrStart( )
{
  if( !pgRefrTmID ) pgRefrTmID = setTimeout('pageRefresh();',5000);
  actEnable('actUpdate',false); actEnable('actStart',false); actEnable('actStop',true);
}
/********************************************************
 * pageCyclRefrStop - Stop curent page cyclic refrash.  *
 ********************************************************/
function pageCyclRefrStop( )
{
  if( pgRefrTmID ) { clearTimeout(pgRefrTmID); pgRefrTmID = null; }
  actEnable('actUpdate',true); actEnable('actStart',true); actEnable('actStop',false);
}
/**************************************************
 * itDBLoad - Load curent page from DB.           *
 **************************************************/
function itDBLoad( )
{
  var rez = servSet(selPath+'/%2fobj','com=com','<load/>',true);
  if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
  else setTimeout('pageRefresh()',500);
}
/**************************************************
 * itDBSave - Save curent page to DB.             *
 **************************************************/
function itDBSave( )
{
  var rez = servSet(selPath+'/%2fobj','com=com','<save/>',true);
  if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
}
/**************************************************
 * pageUp - Select up page.                       *
 **************************************************/
function pageUp( )
{
  var i_l = selPath.length-1;
  while(true)
  {
    var i_l = selPath.lastIndexOf('/',i_l);
    if( i_l == -1 || i_l == 0 ) return;
    if( (selPath.length-i_l) > 1 ) break;
    i_l--;
  }
  selectPage(selPath.substr(0,i_l));
}
/**************************************************
 * pagePrev - Get previous page.                  *
 **************************************************/
function pagePrev( )
{
  if( !ndPrev.length ) return;
  ndNext.push(selPath);
  pageDisplay(ndPrev.pop());
}
/**************************************************
 * pageNext - Get next page.                      *
 **************************************************/
function pageNext( )
{
  if( !ndNext.length ) return;
  ndPrev.push(selPath);
  pageDisplay(ndNext.pop());
}
/**************************************************
 * itAdd - Add new item action process.           *
 **************************************************/
function itAdd( )
{
  if( !selPath.length ) return;
  var branchS = null;
  for( var i_ch = 0; i_ch < root.childNodes.length; i_ch++ )
    if( root.childNodes[i_ch].nodeName.toLowerCase() == 'branches' && root.childNodes[i_ch].getAttribute('id') == 'br' )
      branchS =  root.childNodes[i_ch];
  if( !branchS ) return;

  //> Load branches list
  var typeCfg = '';
  for( var i_b = 0; i_b < branchS.childNodes.length; i_b++ )
    if( parseInt(branchS.childNodes[i_b].getAttribute('acs'))&SEQ_WR )
      typeCfg+="<option idSz='"+branchS.childNodes[i_b].getAttribute('idSz')+
		     "' gid='"+branchS.childNodes[i_b].getAttribute('id')+
		     "' idm='"+(parseInt(branchS.childNodes[i_b].getAttribute('idm'))?"1":"0")+"'>"+
		branchS.childNodes[i_b].getAttribute('dscr')+"</option>";
  if( !typeCfg.length ) { alert('No one editable container present.'); return; }

  dlgWin = ReqIdNameDlg(document.getElementById('actAddIt').childNodes[0].src,'Add item to node:'+selPath);
  dlgWin.document.getElementById('type').style.display = '';
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].innerHTML = typeCfg;
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].onchange = function( )
  {
    if( this.selectedIndex < 0 ) return;
    var fIdVal = dlgWin.document.getElementById('id').childNodes[1].childNodes[0];
    fIdVal.maxLength = this.options[this.selectedIndex].getAttribute('idSz');
    if( !fIdVal.maxLength ) fIdVal.maxLength = 1000;
    dlgWin.document.getElementById('name').style.display = parseInt(this.options[this.selectedIndex].getAttribute('idm'))?'':'none';
  }
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].onchange();
  dlgWin.document.getElementById('actOk').onclick = function()
  {
    var tpSel = dlgWin.document.getElementById('type').childNodes[1].childNodes[0];
    if( !tpSel || tpSel.selectedIndex < 0 ) { dlgWin.close(); return false; }
    var idm = parseInt(tpSel.options[tpSel.selectedIndex].getAttribute('idm'));
    var gbrId = tpSel.options[tpSel.selectedIndex].getAttribute('gid');
    var inpId = dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value;
    var inpName = idm ? dlgWin.document.getElementById('name').childNodes[1].childNodes[0].value : inpId;

    //> Check for already present node
    var req = servSet(selPath+'/%2fbr%2f'+gbrId,'com=com','<get/>',true);
    if( !req || parseInt(req.getAttribute('rez')) != 0 ) { if(req) alert(nodeText(req)); dlgWin.close(); return false; }
    for( var i_lel = 0; i_lel < req.childNodes.length; i_lel++ )
      if( (req.childNodes[i_lel].getAttribute('id') && req.childNodes[i_lel].getAttribute('id') == inpId) ||
	(!req.childNodes[i_lel].getAttribute('id') && nodeText(req.childNodes[i_lel]) == inpId) )
      { alert("Node '"+inpId+"' already present."); dlgWin.close( ); return; }

    //> Send command
    var rez = servSet(selPath+'/%2fbr%2f'+gbrId,'com=com',"<add "+(idm?("id='"+inpId+"'"):"")+">"+inpName+"</add>",true);
    if( !rez || parseInt(rez.getAttribute('rez')) != 0 ) { if(rez) alert(nodeText(rez)); dlgWin.close(); return false; }
    dlgWin.close();
    treeUpdate(); pageRefresh();
    return false;
  }
}
/**************************************************
 * itDel - Delete new item action process.        *
 **************************************************/
function itDel( iit )
{
  var rmit = iit ? iit : selPath;
  if( !rmit || !rmit.length ) return;

  if( !iit && !confirm("You sure for delete node '"+rmit+"'?") ) return;

  var t_el, sel_own = '', sel_el;
  var n_obj = 0;
  for( pathLev.off = 0; (t_el=pathLev(rmit,0,true)).length; n_obj++ )
  { if( n_obj ) sel_own += ('/'+sel_el); sel_el = t_el; }
  if( n_obj > 2 )
  {
    var req = servGet(sel_own+'/%2fbr','com=info');
    if( parseInt(req.getAttribute('rez'))!=0 ) { alert(nodeText(req)); return; }
    if( !req.childNodes[0] ) return;

    var branch = req.childNodes[0];
    for( var i_b = 0; i_b < branch.childNodes.length; i_b++ )
    {
      var b_id = branch.childNodes[i_b].getAttribute('id');
      if( b_id == sel_el.substr(0,b_id.length) && parseInt(branch.childNodes[i_b].getAttribute('acs'))&SEQ_WR )
      {
	var idm = parseInt(branch.childNodes[i_b].getAttribute('idm'));
	var com = idm ? ("<del id='"+sel_el.substr(b_id.length)+"'/>") :
	                ("<del>"+sel_el.substr(b_id.length)+"</del>");
	var rez = servSet(sel_own+'/%2fbr%2f'+b_id,'com=com',com,true);
	if( rez && parseInt(rez.getAttribute('rez')) != 0 ) alert(nodeText(rez));
	else { treeUpdate(); if(!iit) { selPath = sel_own; pageDisplay(selPath); } }
	break;
      }
    }
  }
}
/**************************************************
 * itCut - Cut selected item.                     *
 **************************************************/
function itCut( )
{
  copyBuf = '1'+selPath;
  editToolUpdate();
}
/**************************************************
 * itCopy - Copy selected item.                   *
 **************************************************/
function itCopy( )
{
  copyBuf = '0'+selPath;
  editToolUpdate();
}
/**************************************************
 * itPaste - Paste copy or cut item.              *
 **************************************************/
function itPaste( )
{
  var typeCfg = '';
  var itCnt = 0, defIt = 0;
  var s_el, s_elp = '', t_el, b_grp = '';
  var branchS = null;

  //> Src elements calc
  var n_sel = 0;
  for( pathLev.off = 0; (t_el=pathLev(copyBuf.substr(1),0,true)).length; n_sel++ )
  { if(n_sel) s_elp += ('/'+s_el); s_el = t_el; }

  if( pathLev(copyBuf.substr(1),0) != pathLev(selPath,0) ) { alert('Copy is imposible.'); return; }

  if( parseInt(root.getAttribute('acs'))&SEQ_WR ) { typeCfg+="<option idSz='-1' gid=''>Selected</option>"; itCnt++; }
  for( var i_ch = 0; i_ch < root.childNodes.length; i_ch++ )
    if( root.childNodes[i_ch].nodeName.toLowerCase() == 'branches' && root.childNodes[i_ch].getAttribute('id') == 'br' )
      branchS =  root.childNodes[i_ch];
  if( branchS )
    for( var i_b = 0; i_b < branchS.childNodes.length; i_b++, itCnt++ )
      if( parseInt(branchS.childNodes[i_b].getAttribute('acs'))&SEQ_WR )
      {
	var gbrId = branchS.childNodes[i_b].getAttribute('id');
	typeCfg+="<option idSz='"+branchS.childNodes[i_b].getAttribute('idSz')+"' gid='"+gbrId+"'>"+branchS.childNodes[i_b].getAttribute('dscr')+"</option>";
	if( s_el.substr(0,gbrId.length) == gbrId ) { defIt = itCnt; b_grp = gbrId; }
      }

  //> Make request dialog
  dlgWin = ReqIdNameDlg('/'+MOD_ID+'/img_it_add');
  if( copyBuf.charAt(0) == '1' ) setNodeText(dlgWin.document.getElementById('title').childNodes[1],"Move node '"+copyBuf.substr(1)+" to '"+selPath+"'.");
  else setNodeText(dlgWin.document.getElementById('title').childNodes[1],"Copy node '"+copyBuf.substr(1)+" to '"+selPath+"'.");
  if( b_grp.length ) dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value = s_el.substr(b_grp.length);
  dlgWin.document.getElementById('name').style.display = 'none';
  dlgWin.document.getElementById('type').style.display = '';
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].innerHTML = typeCfg;
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].onchange = function( )
  {
    if( this.selectedIndex < 0 ) return;
    var idSz = parseInt(this.options[this.selectedIndex].getAttribute('idSz'));
    if( idSz < 0 ) dlgWin.document.getElementById('id').style.display = 'none';
    else
    {
      dlgWin.document.getElementById('id').style.display = '';
      var fIdVal = dlgWin.document.getElementById('id').childNodes[1].childNodes[0];
      fIdVal.maxLength = idSz; if( !fIdVal.maxLength ) fIdVal.maxLength = 1000;
    }
  }
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].selectedIndex = defIt;
  dlgWin.document.getElementById('type').childNodes[1].childNodes[0].onchange();
  dlgWin.document.getElementById('actOk').onclick = function()
  {
    var tpSel = dlgWin.document.getElementById('type').childNodes[1].childNodes[0];
    if( !tpSel || tpSel.selectedIndex < 0 ) { dlgWin.close(); return false; }
    var idSz = parseInt(tpSel.options[tpSel.selectedIndex].getAttribute('idSz'));
    var gbrId = tpSel.options[tpSel.selectedIndex].getAttribute('gid');
    var inpId = dlgWin.document.getElementById('id').childNodes[1].childNodes[0].value;

    var statNm, srcNm;
    pathLev.off = 1; statNm = pathLev(copyBuf,0,true); srcNm = copyBuf.substr(pathLev.off);
    pathLev.off = 0; statNm = pathLev(selPath,0,true); dstNm = selPath.substr(pathLev.off);

    if( idSz >= 0 )
    {
      dstNm+='/'+gbrId+inpId;
      //> Check for already present node
      var req = servSet(selPath+'/%2fbr%2f'+gbrId,'com=com','<get/>',true);
      if( !req || parseInt(req.getAttribute('rez')) != 0 ) { if(req) alert(nodeText(req)); dlgWin.close(); return false; }
      for( var i_lel = 0; i_lel < req.childNodes.length; i_lel++ )
	if( (req.childNodes[i_lel].getAttribute('id') && req.childNodes[i_lel].getAttribute('id') == inpId) ||
	    (!req.childNodes[i_lel].getAttribute('id') && nodeText(req.childNodes[i_lel]) == inpId) )
	{
	  if( confirm("Node '"+dstNm+"' already present. Continue?") ) break;
	  dlgWin.close( ); return;
	}
    }
    //> Copy visual item
    var rez = servSet('/'+statNm+'/%2fobj','com=com',"<copy src='"+srcNm+"' dst='"+dstNm+"'/>",true);
    if( !rez || parseInt(rez.getAttribute('rez')) != 0 ) { if(rez) alert(nodeText(rez)); dlgWin.close(); return false; }

    //> Remove source widget
    if( copyBuf.charAt(0) == '1' ) { itDel(copyBuf.substr(1)); copyBuf = '0'; }
    treeUpdate( ); pageRefresh( );
    dlgWin.close( ); return false;
  }
}
/**********************************************************
 * ReqIdNameDlg - Get identifier and name request dialog. *
 **********************************************************/
function ReqIdNameDlg( ico, mess, actPath )
{
  var dlgWin = window.open('','ReqIdNameDlg','width=300,height=180,directories=no,menubar=no,toolbar=no,scrollbars=yes,dependent=yes,location=no,status=no,alwaysRaised=yes');

  dlgWin.document.open();
  dlgWin.document.write(
    "<html><head>\n"+
    " <style type='text/css'>\n"+
    "  table.dlg { width: 98%; border: 3px ridge #FF9253; padding: 5px; text-align: left; vertical-align: top; font-family: Verdana,Arial,Helvetica,sans-serif; font-size:12px; }\n"+
    "  table.dlg select,input { font-family: Verdana,Arial,Helvetica,sans-serif; font-size:12px; }\n"+
    " </style>\n"+
    "</head>\n"+
    "<body style='background-color: #E6E6E6;'>\n"+
    "<center>\n"+
    "<form id='formBlk' action='"+actPath+"' method='post' enctype='multipart/form-data'>\n"+
    "<table border='0' cellspacing='3px' class='dlg'>\n"+
    "<tr><td id='title' colspan='2'><img src='' style='height: 32px; float: left;'/><span/></td></tr>\n"+
    "<tr id='type'><td>Element type:</td><td><select name='type'/></td></tr>\n"+
    "<tr id='id'><td>ID:</td><td><input type='text' name='id'/></td></tr>\n"+
    "<tr id='name'><td>Name:</td><td><input name='name'/></td></tr>\n"+
    "<tr><td colspan='2' style='text-align: right; border-top: 1px solid black; padding-top: 10px;'>\n"+
    "  <input id='actOk' name='actOk' type='button' value='Ok'/> <input id='actCancel' name='actCancel' type='button' value='Close' onclick='window.close(); return false;'/>\n"+
    "</td></tr>\n"+
    "</table>\n"+
    "</form>\n"+
    "</center></body></html>");
  dlgWin.document.close();

  if( ico ) dlgWin.document.getElementById('title').childNodes[0].src = ico;
  if( mess ) setNodeText(dlgWin.document.getElementById('title').childNodes[1],mess);

  var wWidth, wHeight;
  if( window.innerWidth ) { wWidth = dlgWin.outerWidth; wHeight = dlgWin.outerHeight; }
  else { wWidth = dlgWin.document.body.clientWidth; wHeight = dlgWin.document.body.clientHeight; }
  dlgWin.moveTo(Math.round((screen.availWidth-wWidth)/2),Math.round((screen.availHeight-wHeight)/2));

  return dlgWin;
}

//> First start data init
//>> Tool bar init
//>>> Update actions
var actUpdate = document.getElementById('actUpdate');
if( actUpdate ) actUpdate.onclick = function() { if( this.className=='active' ) pageRefresh(); return false; }
var actStart = document.getElementById('actStart');
if( actStart ) actStart.onclick = function() { if( this.className=='active' ) pageCyclRefrStart(); return false; }
var actStop = document.getElementById('actStop');
if( actStop ) actStop.onclick = function() { if( this.className=='active' ) pageCyclRefrStop(); return false; }
//>>> DB actions
var actLoad = document.getElementById('actLoad');
if( actLoad ) actLoad.onclick = function() { if( this.className=='active' ) itDBLoad(); return false; }
var actSave = document.getElementById('actSave');
if( actSave ) actSave.onclick = function() { if( this.className=='active' ) itDBSave(); return false; }
//>>> Navigate actions
var actUp = document.getElementById('actUp');
if( actUp ) actUp.onclick = function() { if( this.className=='active' ) pageUp(); return false; }
var actPrevious = document.getElementById('actPrevious');
if( actPrevious ) actPrevious.onclick = function() { if( this.className=='active' ) pagePrev(); return false; }
var actNext = document.getElementById('actNext');
if( actNext ) actNext.onclick = function() { if( this.className=='active' ) pageNext(); return false; }
//>>> Create/delete actions
var actAddIt = document.getElementById('actAddIt');
if( actAddIt ) actAddIt.onclick = function() { if( this.className=='active' ) itAdd(); return false; }
var actDelIt = document.getElementById('actDelIt');
if( actDelIt ) actDelIt.onclick = function() { if( this.className=='active' ) itDel(); return false; }
//>>> Copy actions
var actCopy = document.getElementById('actCopy');
if( actCopy ) actCopy.onclick = function() { if( this.className=='active' ) itCopy(); return false; }
var actCut = document.getElementById('actCut');
if( actCut ) actCut.onclick = function() { if( this.className=='active' ) itCut(); return false; }
var actPaste = document.getElementById('actPaste');
if( actPaste ) actPaste.onclick = function() { if( this.className=='active' ) itPaste(); return false; }

pageDisplay(hostsUpdate());

setStatus('Page loaded.',5000);

<!DOCTYPE html>
<!--[if lt IE 7 ]> <html lang="en" class="no-js ie6"> <![endif]-->
<!--[if IE 7 ]>    <html lang="en" class="no-js ie7"> <![endif]-->
<!--[if IE 8 ]>    <html lang="en" class="no-js ie8"> <![endif]-->
<!--[if IE 9 ]>    <html lang="en" class="no-js ie9"> <![endif]-->
<!--[if (gt IE 9)|!(IE)]><!--><html lang="en" class="no-js"><!--<![endif]-->
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
	<title><g:layoutTitle default="Grails"/></title>
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<link rel="shortcut icon" href="${resource(dir: 'images', file: 'favicon.ico')}" type="image/x-icon">
	<link rel="apple-touch-icon" href="${resource(dir: 'images', file: 'apple-touch-icon.png')}">
	<link rel="apple-touch-icon" sizes="114x114" href="${resource(dir: 'images', file: 'apple-touch-icon-retina.png')}">
	<link rel="stylesheet" href="${resource(dir: 'css', file: 'main.css')}" type="text/css">
	<link rel="stylesheet" href="${resource(dir: 'css', file: 'mobile.css')}" type="text/css">
    <g:javascript library="jquery"/>
	<r:layoutResources />
	<g:layoutHead/>
</head>
<body>
	<div id="grailsLogo" role="banner"><p style="line-height:3em;font-size:1.5em;padding-left:1em;">流媒体平台管理器</p></div>
    <div class="nav" role="navigation">
        <ul>
            <li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
            <li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
            <li><g:link class="list" action="list" controller="mediaEvent"><g:message code="default.list.label" args="[message(code: 'mediaEvent.label', default: 'Media Event')]" /></g:link></li>
            <li><g:link class="create" action="create"><g:message code="default.new.label" args="[message(code: 'mediaService.label', default: 'Media Service')]" /></g:link></li>
        </ul>
    </div>
    <div class="body">
	<g:layoutBody/>
    </div>
	<div class="footer" role="contentinfo"></div>
	<div id="spinner" class="spinner" style="display:none;"><g:message code="spinner.alt" default="Loading&hellip;"/></div>
	<g:javascript library="application"/>
	<r:layoutResources />
</body>
</html>

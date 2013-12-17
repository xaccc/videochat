
<%@ page import="videochat.Online" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'online.label', default: 'Online')}" />
		<title><g:message code="default.show.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#show-online" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
				<li><g:link class="list" action="list"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
				<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
			</ul>
		</div>
		<div id="show-online" class="content scaffold-show" role="main">
			<h1><g:message code="default.show.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<ol class="property-list online">
			
				<g:if test="${onlineInstance?.uid}">
				<li class="fieldcontain">
					<span id="uid-label" class="property-label"><g:message code="online.uid.label" default="Uid" /></span>
					
						<span class="property-value" aria-labelledby="uid-label"><g:fieldValue bean="${onlineInstance}" field="uid"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${onlineInstance?.mediaServiceId}">
				<li class="fieldcontain">
					<span id="mediaServiceId-label" class="property-label"><g:message code="online.mediaServiceId.label" default="Media Service Id" /></span>
					
						<span class="property-value" aria-labelledby="mediaServiceId-label"><g:fieldValue bean="${onlineInstance}" field="mediaServiceId"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${onlineInstance?.sessionId}">
				<li class="fieldcontain">
					<span id="sessionId-label" class="property-label"><g:message code="online.sessionId.label" default="Session Id" /></span>
					
						<span class="property-value" aria-labelledby="sessionId-label"><g:fieldValue bean="${onlineInstance}" field="sessionId"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${onlineInstance?.dateCreated}">
				<li class="fieldcontain">
					<span id="dateCreated-label" class="property-label"><g:message code="online.dateCreated.label" default="Date Created" /></span>
					
						<span class="property-value" aria-labelledby="dateCreated-label"><g:formatDate date="${onlineInstance?.dateCreated}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${onlineInstance?.lastUpdated}">
				<li class="fieldcontain">
					<span id="lastUpdated-label" class="property-label"><g:message code="online.lastUpdated.label" default="Last Updated" /></span>
					
						<span class="property-value" aria-labelledby="lastUpdated-label"><g:formatDate date="${onlineInstance?.lastUpdated}" /></span>
					
				</li>
				</g:if>
			
			</ol>
			<g:form>
				<fieldset class="buttons">
					<g:hiddenField name="id" value="${onlineInstance?.id}" />
					<g:link class="edit" action="edit" id="${onlineInstance?.id}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
					<g:actionSubmit class="delete" action="delete" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
				</fieldset>
			</g:form>
		</div>
	</body>
</html>

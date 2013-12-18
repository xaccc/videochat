
<%@ page import="videochat.MediaEvent" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaEvent.label', default: 'MediaEvent')}" />
		<title><g:message code="default.show.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#show-mediaEvent" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
				<li><g:link class="list" action="list"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
				<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
			</ul>
		</div>
		<div id="show-mediaEvent" class="content scaffold-show" role="main">
			<h1><g:message code="default.show.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<ol class="property-list mediaEvent">
			
				<g:if test="${mediaEventInstance?.uid}">
				<li class="fieldcontain">
					<span id="uid-label" class="property-label"><g:message code="mediaEvent.uid.label" default="Uid" /></span>
					
						<span class="property-value" aria-labelledby="uid-label"><g:fieldValue bean="${mediaEventInstance}" field="uid"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaEventInstance?.sessionId}">
				<li class="fieldcontain">
					<span id="sessionId-label" class="property-label"><g:message code="mediaEvent.sessionId.label" default="Session Id" /></span>
					
						<span class="property-value" aria-labelledby="sessionId-label"><g:fieldValue bean="${mediaEventInstance}" field="sessionId"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaEventInstance?.mediaServiceId}">
				<li class="fieldcontain">
					<span id="mediaServiceId-label" class="property-label"><g:message code="mediaEvent.mediaServiceId.label" default="Media Service Id" /></span>
					
						<span class="property-value" aria-labelledby="mediaServiceId-label"><g:fieldValue bean="${mediaEventInstance}" field="mediaServiceId"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaEventInstance?.dateLive}">
				<li class="fieldcontain">
					<span id="dateLive-label" class="property-label"><g:message code="mediaEvent.dateLive.label" default="Date Live" /></span>
					
						<span class="property-value" aria-labelledby="dateLive-label"><g:formatDate date="${mediaEventInstance?.dateLive}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaEventInstance?.dateLeave}">
				<li class="fieldcontain">
					<span id="dateLeave-label" class="property-label"><g:message code="mediaEvent.dateLeave.label" default="Date Leave" /></span>
					
						<span class="property-value" aria-labelledby="dateLeave-label"><g:formatDate date="${mediaEventInstance?.dateLeave}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaEventInstance?.duration}">
				<li class="fieldcontain">
					<span id="duration-label" class="property-label"><g:message code="mediaEvent.duration.label" default="Duration" /></span>
					
						<span class="property-value" aria-labelledby="duration-label"><g:fieldValue bean="${mediaEventInstance}" field="duration"/></span>
					
				</li>
				</g:if>
			
			</ol>
			<g:form>
				<fieldset class="buttons">
					<g:hiddenField name="id" value="${mediaEventInstance?.id}" />
					<g:link class="edit" action="edit" id="${mediaEventInstance?.id}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
					<g:actionSubmit class="delete" action="delete" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
				</fieldset>
			</g:form>
		</div>
	</body>
</html>

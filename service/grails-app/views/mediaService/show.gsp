
<%@ page import="videochat.MediaService" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaService.label', default: 'MediaService')}" />
		<title><g:message code="default.show.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#show-mediaService" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaBackup"><g:message code="default.list.label" args="[message(code: 'mediaBackup.label', default: 'Backups')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
				<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
			</ul>
		</div>
		<div id="show-mediaService" class="content scaffold-show" role="main">
			<h1><g:message code="default.show.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<ol class="property-list mediaService">
			
				<g:if test="${mediaServiceInstance?.name}">
				<li class="fieldcontain">
					<span id="name-label" class="property-label"><g:message code="mediaService.name.label" default="Name" /></span>
					
						<span class="property-value" aria-labelledby="name-label"><g:fieldValue bean="${mediaServiceInstance}" field="name"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.scheme}">
				<li class="fieldcontain">
					<span id="scheme-label" class="property-label"><g:message code="mediaService.scheme.label" default="Scheme" /></span>
					
						<span class="property-value" aria-labelledby="scheme-label"><g:fieldValue bean="${mediaServiceInstance}" field="scheme"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.domain}">
				<li class="fieldcontain">
					<span id="domain-label" class="property-label"><g:message code="mediaService.domain.label" default="Domain" /></span>
					
						<span class="property-value" aria-labelledby="domain-label"><g:fieldValue bean="${mediaServiceInstance}" field="domain"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.port}">
				<li class="fieldcontain">
					<span id="port-label" class="property-label"><g:message code="mediaService.port.label" default="Port" /></span>
					
						<span class="property-value" aria-labelledby="port-label">${mediaServiceInstance.port}</span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.path}">
				<li class="fieldcontain">
					<span id="path-label" class="property-label"><g:message code="mediaService.path.label" default="Path" /></span>
					
						<span class="property-value" aria-labelledby="path-label"><g:fieldValue bean="${mediaServiceInstance}" field="path"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.dateCreated}">
				<li class="fieldcontain">
					<span id="dateCreated-label" class="property-label"><g:message code="mediaService.dateCreated.label" default="Date Created" /></span>
					
						<span class="property-value" aria-labelledby="dateCreated-label"><g:formatDate date="${mediaServiceInstance?.dateCreated}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaServiceInstance?.lastUpdated}">
				<li class="fieldcontain">
					<span id="lastUpdated-label" class="property-label"><g:message code="mediaService.lastUpdated.label" default="Last Updated" /></span>
					
						<span class="property-value" aria-labelledby="lastUpdated-label"><g:formatDate date="${mediaServiceInstance?.lastUpdated}" /></span>
					
				</li>
				</g:if>
			
			</ol>
			<g:form>
				<fieldset class="buttons">
					<g:hiddenField name="id" value="${mediaServiceInstance?.id}" />
					<g:link class="edit" action="edit" id="${mediaServiceInstance?.id}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
					<g:actionSubmit class="delete" action="delete" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
				</fieldset>
			</g:form>
		</div>
	</body>
</html>

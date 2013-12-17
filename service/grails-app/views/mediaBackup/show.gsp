
<%@ page import="videochat.MediaBackup" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaBackup.label', default: 'MediaBackup')}" />
		<title><g:message code="default.show.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#show-mediaBackup" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></li>
				<li><g:link class="list" action="list"><g:message code="default.list.label" args="[entityName]" /></g:link></li>
				<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
			</ul>
		</div>
		<div id="show-mediaBackup" class="content scaffold-show" role="main">
			<h1><g:message code="default.show.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<ol class="property-list mediaBackup">
			
				<g:if test="${mediaBackupInstance?.uid}">
				<li class="fieldcontain">
					<span id="uid-label" class="property-label"><g:message code="mediaBackup.uid.label" default="Uid" /></span>
					
						<span class="property-value" aria-labelledby="uid-label"><g:fieldValue bean="${mediaBackupInstance}" field="uid"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.domain}">
				<li class="fieldcontain">
					<span id="domain-label" class="property-label"><g:message code="mediaBackup.domain.label" default="Domain" /></span>
					
						<span class="property-value" aria-labelledby="domain-label"><g:fieldValue bean="${mediaBackupInstance}" field="domain"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.sessionId}">
				<li class="fieldcontain">
					<span id="sessionId-label" class="property-label"><g:message code="mediaBackup.sessionId.label" default="Session Id" /></span>
					
						<span class="property-value" aria-labelledby="sessionId-label"><g:fieldValue bean="${mediaBackupInstance}" field="sessionId"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.flvid}">
				<li class="fieldcontain">
					<span id="flvid-label" class="property-label"><g:message code="mediaBackup.flvid.label" default="Flvid" /></span>
					
						<span class="property-value" aria-labelledby="flvid-label"><g:fieldValue bean="${mediaBackupInstance}" field="flvid"/></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.first}">
				<li class="fieldcontain">
					<span id="first-label" class="property-label"><g:message code="mediaBackup.first.label" default="First" /></span>
					
						<span class="property-value" aria-labelledby="first-label"><g:formatBoolean boolean="${mediaBackupInstance?.first}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.backupDate}">
				<li class="fieldcontain">
					<span id="backupDate-label" class="property-label"><g:message code="mediaBackup.backupDate.label" default="Backup Date" /></span>
					
						<span class="property-value" aria-labelledby="backupDate-label"><g:formatDate date="${mediaBackupInstance?.backupDate}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.dateCreated}">
				<li class="fieldcontain">
					<span id="dateCreated-label" class="property-label"><g:message code="mediaBackup.dateCreated.label" default="Date Created" /></span>
					
						<span class="property-value" aria-labelledby="dateCreated-label"><g:formatDate date="${mediaBackupInstance?.dateCreated}" /></span>
					
				</li>
				</g:if>
			
				<g:if test="${mediaBackupInstance?.lastUpdated}">
				<li class="fieldcontain">
					<span id="lastUpdated-label" class="property-label"><g:message code="mediaBackup.lastUpdated.label" default="Last Updated" /></span>
					
						<span class="property-value" aria-labelledby="lastUpdated-label"><g:formatDate date="${mediaBackupInstance?.lastUpdated}" /></span>
					
				</li>
				</g:if>
			
			</ol>
			<g:form>
				<fieldset class="buttons">
					<g:hiddenField name="id" value="${mediaBackupInstance?.id}" />
					<g:link class="edit" action="edit" id="${mediaBackupInstance?.id}"><g:message code="default.button.edit.label" default="Edit" /></g:link>
					<g:actionSubmit class="delete" action="delete" value="${message(code: 'default.button.delete.label', default: 'Delete')}" onclick="return confirm('${message(code: 'default.button.delete.confirm.message', default: 'Are you sure?')}');" />
				</fieldset>
			</g:form>
		</div>
	</body>
</html>

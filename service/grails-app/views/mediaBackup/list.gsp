
<%@ page import="videochat.MediaBackup" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaBackup.label', default: 'MediaBackup')}" />
		<title><g:message code="default.list.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#list-mediaBackup" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaBackup"><g:message code="default.list.label" args="[message(code: 'mediaBackup.label', default: 'Backups')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaEvent"><g:message code="default.list.label" args="[message(code: 'mediaEvent.label', default: 'Media Event')]" /></g:link></li>
			</ul>
		</div>
		<div id="list-mediaBackup" class="content scaffold-list" role="main">
			<h1><g:message code="default.list.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<table>
				<thead>
					<tr>
					
						<g:sortableColumn property="uid" title="${message(code: 'mediaBackup.uid.label', default: 'Uid')}" />
					
						<g:sortableColumn property="domain" title="${message(code: 'mediaBackup.domain.label', default: 'Domain')}" />
					
						<g:sortableColumn property="sessionId" title="${message(code: 'mediaBackup.sessionId.label', default: 'Session Id')}" />
					
						<g:sortableColumn property="flvid" title="${message(code: 'mediaBackup.flvid.label', default: 'Flvid')}" />
					
						<g:sortableColumn property="first" title="${message(code: 'mediaBackup.first.label', default: 'First')}" />
					
						<g:sortableColumn property="backupDate" title="${message(code: 'mediaBackup.backupDate.label', default: 'Backup Date')}" />
					
					</tr>
				</thead>
				<tbody>
				<g:each in="${mediaBackupInstanceList}" status="i" var="mediaBackupInstance">
					<tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "uid")}</td>
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "domain")}</td>
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "sessionId")}</td>
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "flvid")}</td>
					
						<td><g:formatBoolean boolean="${mediaBackupInstance.first}" /></td>
					
						<td><g:formatDate date="${mediaBackupInstance.backupDate}" /></td>
					
					</tr>
				</g:each>
				</tbody>
			</table>
			<div class="pagination">
				<g:paginate total="${mediaBackupInstanceTotal}" />
			</div>
		</div>
	</body>
</html>

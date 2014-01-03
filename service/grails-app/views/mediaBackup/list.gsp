
<%@ page import="videochat.MediaBackup" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaBackup.label', default: 'MediaBackup')}" />
		<title><g:message code="default.list.label" args="[entityName]" /></title>
		<script src="flowplayer-3.2.13.min.js"></script>
		<script language="JavaScript">
			flowplayer("player", "path/to/the/flowplayer-3.2.18.swf");
		</script>
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

						<th>操作</th>
					
					</tr>
				</thead>
				<tbody>
				<g:each in="${mediaBackupInstanceList}" status="i" var="mediaBackupInstance">
					<tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
					
						<td><g:link controller="mediaBackup" action="listByUid" id="${mediaBackupInstance.uid}">${fieldValue(bean: mediaBackupInstance, field: "uid")}</g:link></td>
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "domain")}</td>
					
						<td><g:link controller="mediaBackup" action="listBySession" id="${mediaBackupInstance.sessionId}">${fieldValue(bean: mediaBackupInstance, field: "sessionId")}</g:link></td>
					
						<td>${fieldValue(bean: mediaBackupInstance, field: "flvid")}</td>
					
						<td><g:formatBoolean boolean="${mediaBackupInstance.first}" /></td>
					
						<td><g:formatDate date="${mediaBackupInstance.backupDate}" /></td>

						<td>查看</td>
					
					</tr>
				</g:each>
				</tbody>
			</table>
			<div class="pagination">
				<g:paginate total="${mediaBackupInstanceTotal}" params="[id:params.id]" />
			</div>
		</div>
		<div class="flowplayer">
		   <video>
		      <source type="video/webm" src="http://mydomain.com/path/to/intro.webm">
		      <source type="video/mp4"  src="http://mydomain.com/path/to/intro.mp4">
		   </video>
		</div>
	</body>
</html>

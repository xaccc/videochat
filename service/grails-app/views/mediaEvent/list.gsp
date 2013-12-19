
<%@ page import="videochat.MediaEvent" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaEvent.label', default: 'MediaEvent')}" />
		<title><g:message code="default.list.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#list-mediaEvent" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaBackup"><g:message code="default.list.label" args="[message(code: 'mediaBackup.label', default: 'Backups')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaEvent"><g:message code="default.list.label" args="[message(code: 'mediaEvent.label', default: 'Media Event')]" /></g:link></li>
			</ul>
		</div>
		<div id="list-mediaEvent" class="content scaffold-list" role="main">
			<h1><g:message code="default.list.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<table>
				<thead>
					<tr>
					
						<g:sortableColumn property="uid" title="${message(code: 'mediaEvent.uid.label', default: 'Uid')}" />
					
						<g:sortableColumn property="mediaServiceId" title="${message(code: 'mediaEvent.mediaServiceId.label', default: 'Media Service Id')}" />
					
						<g:sortableColumn property="sessionId" title="${message(code: 'mediaEvent.sessionId.label', default: 'Session Id')}" />
					
						<g:sortableColumn property="dateLive" title="${message(code: 'mediaEvent.dateLive.label', default: 'Date Live')}" />
					
						<g:sortableColumn property="dateLeave" title="${message(code: 'mediaEvent.dateLeave.label', default: 'Date Leave')}" />
					
						<g:sortableColumn property="duration" title="${message(code: 'mediaEvent.duration.label', default: 'Duration')}" />
					
					</tr>
				</thead>
				<tbody>
				<g:each in="${mediaEventInstanceList}" status="i" var="mediaEventInstance">
                    <g:set var="mediaService" value="${videochat.MediaService.get(mediaEventInstance.mediaServiceId)}" />
					<tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
					
						<td uuid="${mediaEventInstance?.id}"><g:link controller="mediaBackup" action="listByUid" id="${mediaEventInstance.uid}">${fieldValue(bean: mediaEventInstance, field: "uid")}</g:link></td>
					
                        <g:if test="${mediaService}">
                        <td><g:link action="show" id="${mediaService?.id}" controller="mediaService">${fieldValue(bean: mediaService, field: "name")}</g:link></td>
                        </g:if>
                        <g:else>
						<td>${fieldValue(bean: mediaEventInstance, field: "mediaServiceId")}</td>
                        </g:else>
					
						<td><g:link controller="mediaBackup" action="listBySession" id="${mediaEventInstance.sessionId}">${fieldValue(bean: mediaEventInstance, field: "sessionId")}</g:link></td>
					
						<td><g:formatDate date="${mediaEventInstance.dateLive}" /></td>
					
						<td><g:formatDate date="${mediaEventInstance.dateLeave}" /></td>
					
						<td>${fieldValue(bean: mediaEventInstance, field: "duration")}分钟</td>
					
					</tr>
				</g:each>
				</tbody>
			</table>
			<div class="pagination">
				<g:paginate total="${mediaEventInstanceTotal}" />
			</div>
		</div>
	</body>
</html>

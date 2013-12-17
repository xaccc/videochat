
<%@ page import="videochat.MediaService" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaService.label', default: 'MediaService')}" />
		<title><g:message code="default.list.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#list-mediaService" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaBackup"><g:message code="default.list.label" args="[message(code: 'mediaBackup.label', default: 'Backups')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
				<li><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></li>
			</ul>
		</div>
		<div id="list-mediaService" class="content scaffold-list" role="main">
			<h1><g:message code="default.list.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<table>
				<thead>
					<tr>
					
						<g:sortableColumn property="name" title="${message(code: 'mediaService.name.label', default: 'Name')}" />
					
						<g:sortableColumn property="scheme" title="${message(code: 'mediaService.scheme.label', default: 'Scheme')}" />
					
						<g:sortableColumn property="domain" title="${message(code: 'mediaService.domain.label', default: 'Domain')}" />
					
						<g:sortableColumn property="port" title="${message(code: 'mediaService.port.label', default: 'Port')}" />
					
						<g:sortableColumn property="path" title="${message(code: 'mediaService.path.label', default: 'Path')}" />
					
						<g:sortableColumn property="dateCreated" title="${message(code: 'mediaService.dateCreated.label', default: 'Date Created')}" />
					
					</tr>
				</thead>
				<tbody>
				<g:each in="${mediaServiceInstanceList}" status="i" var="mediaServiceInstance">
					<tr class="${(i % 2) == 0 ? 'even' : 'odd'}">
					
						<td><g:link action="show" id="${mediaServiceInstance.id}">${fieldValue(bean: mediaServiceInstance, field: "name")}</g:link></td>
					
						<td>${fieldValue(bean: mediaServiceInstance, field: "scheme")}</td>
					
						<td>${fieldValue(bean: mediaServiceInstance, field: "domain")}</td>
					
						<td>${fieldValue(bean: mediaServiceInstance, field: "port")}</td>
					
						<td>${fieldValue(bean: mediaServiceInstance, field: "path")}</td>
					
						<td><g:formatDate date="${mediaServiceInstance.dateCreated}" /></td>
					
					</tr>
				</g:each>
				</tbody>
			</table>
			<div class="pagination">
				<g:paginate total="${mediaServiceInstanceTotal}" />
			</div>
		</div>
	</body>
</html>

package videochat

import org.springframework.dao.DataIntegrityViolationException

class OnlineController {

    static allowedMethods = [save: "POST", update: "POST", delete: "POST"]

    def index() {
        redirect(action: "list", params: params)
    }

    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        [onlineInstanceList: Online.list(params), onlineInstanceTotal: Online.count()]
    }

    def create() {
        [onlineInstance: new Online(params)]
    }

    def save() {
        def onlineInstance = new Online(params)
        if (!onlineInstance.save(flush: true)) {
            render(view: "create", model: [onlineInstance: onlineInstance])
            return
        }

        flash.message = message(code: 'default.created.message', args: [message(code: 'online.label', default: 'Online'), onlineInstance.id])
        redirect(action: "show", id: onlineInstance.id)
    }

    def show(String id) {
        def onlineInstance = Online.get(id)
        if (!onlineInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "list")
            return
        }

        [onlineInstance: onlineInstance]
    }

    def edit(String id) {
        def onlineInstance = Online.get(id)
        if (!onlineInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "list")
            return
        }

        [onlineInstance: onlineInstance]
    }

    def update(String id, Long version) {
        def onlineInstance = Online.get(id)
        if (!onlineInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "list")
            return
        }

        if (version != null) {
            if (onlineInstance.version > version) {
                onlineInstance.errors.rejectValue("version", "default.optimistic.locking.failure",
                          [message(code: 'online.label', default: 'Online')] as Object[],
                          "Another user has updated this Online while you were editing")
                render(view: "edit", model: [onlineInstance: onlineInstance])
                return
            }
        }

        onlineInstance.properties = params

        if (!onlineInstance.save(flush: true)) {
            render(view: "edit", model: [onlineInstance: onlineInstance])
            return
        }

        flash.message = message(code: 'default.updated.message', args: [message(code: 'online.label', default: 'Online'), onlineInstance.id])
        redirect(action: "show", id: onlineInstance.id)
    }

    def delete(String id) {
        def onlineInstance = Online.get(id)
        if (!onlineInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "list")
            return
        }

        try {
            onlineInstance.delete(flush: true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "list")
        }
        catch (DataIntegrityViolationException e) {
            flash.message = message(code: 'default.not.deleted.message', args: [message(code: 'online.label', default: 'Online'), id])
            redirect(action: "show", id: id)
        }
    }
}

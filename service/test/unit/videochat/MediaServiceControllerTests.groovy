package videochat



import org.junit.*
import grails.test.mixin.*

@TestFor(MediaServiceController)
@Mock(MediaService)
class MediaServiceControllerTests {

    def populateValidParams(params) {
        assert params != null
        // TODO: Populate valid properties like...
        //params["name"] = 'someValidName'
    }

    void testIndex() {
        controller.index()
        assert "/mediaService/list" == response.redirectedUrl
    }

    void testList() {

        def model = controller.list()

        assert model.mediaServiceInstanceList.size() == 0
        assert model.mediaServiceInstanceTotal == 0
    }

    void testCreate() {
        def model = controller.create()

        assert model.mediaServiceInstance != null
    }

    void testSave() {
        controller.save()

        assert model.mediaServiceInstance != null
        assert view == '/mediaService/create'

        response.reset()

        populateValidParams(params)
        controller.save()

        assert response.redirectedUrl == '/mediaService/show/1'
        assert controller.flash.message != null
        assert MediaService.count() == 1
    }

    void testShow() {
        controller.show()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaService/list'

        populateValidParams(params)
        def mediaService = new MediaService(params)

        assert mediaService.save() != null

        params.id = mediaService.id

        def model = controller.show()

        assert model.mediaServiceInstance == mediaService
    }

    void testEdit() {
        controller.edit()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaService/list'

        populateValidParams(params)
        def mediaService = new MediaService(params)

        assert mediaService.save() != null

        params.id = mediaService.id

        def model = controller.edit()

        assert model.mediaServiceInstance == mediaService
    }

    void testUpdate() {
        controller.update()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaService/list'

        response.reset()

        populateValidParams(params)
        def mediaService = new MediaService(params)

        assert mediaService.save() != null

        // test invalid parameters in update
        params.id = mediaService.id
        //TODO: add invalid values to params object

        controller.update()

        assert view == "/mediaService/edit"
        assert model.mediaServiceInstance != null

        mediaService.clearErrors()

        populateValidParams(params)
        controller.update()

        assert response.redirectedUrl == "/mediaService/show/$mediaService.id"
        assert flash.message != null

        //test outdated version number
        response.reset()
        mediaService.clearErrors()

        populateValidParams(params)
        params.id = mediaService.id
        params.version = -1
        controller.update()

        assert view == "/mediaService/edit"
        assert model.mediaServiceInstance != null
        assert model.mediaServiceInstance.errors.getFieldError('version')
        assert flash.message != null
    }

    void testDelete() {
        controller.delete()
        assert flash.message != null
        assert response.redirectedUrl == '/mediaService/list'

        response.reset()

        populateValidParams(params)
        def mediaService = new MediaService(params)

        assert mediaService.save() != null
        assert MediaService.count() == 1

        params.id = mediaService.id

        controller.delete()

        assert MediaService.count() == 0
        assert MediaService.get(mediaService.id) == null
        assert response.redirectedUrl == '/mediaService/list'
    }
}

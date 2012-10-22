// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCTiledLayerTestCommon_h
#define CCTiledLayerTestCommon_h

#include "CCGraphicsContext.h"
#include "IntRect.h"
#include "IntSize.h"
#include "Region.h"
#include "cc/layer_texture_updater.h"
#include "cc/prioritized_texture.h"
#include "cc/resource_provider.h"
#include "cc/texture_copier.h"
#include "cc/texture_update_queue.h"
#include "cc/texture_uploader.h"
#include "cc/tiled_layer.h"
#include "cc/tiled_layer_impl.h"

namespace WebKitTests {

class FakeTiledLayer;

class FakeLayerTextureUpdater : public cc::LayerTextureUpdater {
public:
    class Texture : public cc::LayerTextureUpdater::Texture {
    public:
        Texture(FakeLayerTextureUpdater*, scoped_ptr<cc::PrioritizedTexture>);
        virtual ~Texture();

        virtual void update(cc::TextureUpdateQueue&, const cc::IntRect&, const cc::IntSize&, bool, cc::RenderingStats&) OVERRIDE;

    private:
        FakeLayerTextureUpdater* m_layer;
        SkBitmap m_bitmap;
    };

    FakeLayerTextureUpdater();

    virtual scoped_ptr<cc::LayerTextureUpdater::Texture> createTexture(cc::PrioritizedTextureManager*) OVERRIDE;

    virtual void prepareToUpdate(const cc::IntRect& contentRect, const cc::IntSize&, float, float, cc::IntRect& resultingOpaqueRect, cc::RenderingStats&) OVERRIDE;
    // Sets the rect to invalidate during the next call to prepareToUpdate(). After the next
    // call to prepareToUpdate() the rect is reset.
    void setRectToInvalidate(const cc::IntRect&, FakeTiledLayer*);
    // Last rect passed to prepareToUpdate().
    const cc::IntRect& lastUpdateRect()  const { return m_lastUpdateRect; }

    // Number of times prepareToUpdate has been invoked.
    int prepareCount() const { return m_prepareCount; }
    void clearPrepareCount() { m_prepareCount = 0; }

    // Number of times update() has been invoked on a texture.
    int updateCount() const { return m_updateCount; }
    void clearUpdateCount() { m_updateCount = 0; }
    void update() { m_updateCount++; }

    void setOpaquePaintRect(const cc::IntRect& opaquePaintRect) { m_opaquePaintRect = opaquePaintRect; }

protected:
    virtual ~FakeLayerTextureUpdater();

private:
    int m_prepareCount;
    int m_updateCount;
    cc::IntRect m_rectToInvalidate;
    cc::IntRect m_lastUpdateRect;
    cc::IntRect m_opaquePaintRect;
    scoped_refptr<FakeTiledLayer> m_layer;
};

class FakeTiledLayerImpl : public cc::TiledLayerImpl {
public:
    explicit FakeTiledLayerImpl(int id);
    virtual ~FakeTiledLayerImpl();

    using cc::TiledLayerImpl::hasTileAt;
    using cc::TiledLayerImpl::hasResourceIdForTileAt;
};

class FakeTiledLayer : public cc::TiledLayer {
public:
    explicit FakeTiledLayer(cc::PrioritizedTextureManager*);

    static cc::IntSize tileSize() { return cc::IntSize(100, 100); }

    using cc::TiledLayer::invalidateContentRect;
    using cc::TiledLayer::needsIdlePaint;
    using cc::TiledLayer::skipsDraw;
    using cc::TiledLayer::numPaintedTiles;
    using cc::TiledLayer::idlePaintRect;

    virtual void setNeedsDisplayRect(const cc::FloatRect&) OVERRIDE;
    const cc::FloatRect& lastNeedsDisplayRect() const { return m_lastNeedsDisplayRect; }

    virtual void setTexturePriorities(const cc::PriorityCalculator&) OVERRIDE;

    virtual cc::PrioritizedTextureManager* textureManager() const OVERRIDE;
    FakeLayerTextureUpdater* fakeLayerTextureUpdater() { return m_fakeTextureUpdater.get(); }
    cc::FloatRect updateRect() { return m_updateRect; }

protected:
    virtual cc::LayerTextureUpdater* textureUpdater() const OVERRIDE;
    virtual void createTextureUpdaterIfNeeded() OVERRIDE { }
    virtual ~FakeTiledLayer();

private:
    scoped_refptr<FakeLayerTextureUpdater> m_fakeTextureUpdater;
    cc::PrioritizedTextureManager* m_textureManager;
    cc::FloatRect m_lastNeedsDisplayRect;
};

class FakeTiledLayerWithScaledBounds : public FakeTiledLayer {
public:
    explicit FakeTiledLayerWithScaledBounds(cc::PrioritizedTextureManager*);

    void setContentBounds(const cc::IntSize& contentBounds) { m_forcedContentBounds = contentBounds; }
    virtual cc::IntSize contentBounds() const OVERRIDE;

protected:
    virtual ~FakeTiledLayerWithScaledBounds();
    cc::IntSize m_forcedContentBounds;
};

}
#endif // CCTiledLayerTestCommon_h

/*
 * Copyright (c) 2010-2020 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <client/painter/creaturepainter.h>
#include <client/map/map.h>
#include <client/game.h>

#include <framework/core/declarations.h>
#include <framework/graphics/framebuffermanager.h>
#include <framework/graphics/graphics.h>

#include "framework/stdext/math.h"

void CreaturePainter::draw(const CreaturePtr& creature, const Point& dest, float scaleFactor, const Highlight& highLight, int frameFlags, LightView* lightView)
{
    if(!creature->canBeSeen())
        return;

    if(frameFlags & Otc::FUpdateThing) {
        if(creature->m_showTimedSquare) {
            g_painter->setColor(creature->m_timedSquareColor);
            g_painter->drawBoundingRect(Rect(dest + (creature->m_walkOffset - creature->getDisplacement() + 2) * scaleFactor, Size((SPRITE_SIZE - 4) * scaleFactor)), std::max<int>(static_cast<int>(2 * scaleFactor), 1));
            g_painter->resetColor();
        }

        if(creature->m_showStaticSquare) {
            g_painter->setColor(creature->m_staticSquareColor);
            g_painter->drawBoundingRect(Rect(dest + (creature->m_walkOffset - creature->getDisplacement()) * scaleFactor, Size(SPRITE_SIZE * scaleFactor)), std::max<int>(static_cast<int>(2 * scaleFactor), 1));
            g_painter->resetColor();
        }

        internalDrawOutfit(creature, dest + (creature->m_walkOffset * scaleFactor), scaleFactor, false, creature->m_direction);

        if(highLight.enabled && creature == highLight.thing) {
            g_painter->setColor(highLight.rgbColor);
            internalDrawOutfit(creature, dest + (creature->m_walkOffset * scaleFactor), scaleFactor, true, creature->m_direction);
            g_painter->resetColor();
        }
    }

    if(lightView && frameFlags & Otc::FUpdateLight) {
        auto light = creature->getLight();

        if(creature->isLocalPlayer() && (g_map.getLight().intensity < 64 || creature->m_position.z > SEA_FLOOR)) {
            if(light.intensity == 0) {
                light.intensity = 2;
                light.brightness = .2f;
            } else if(light.color == 0 || light.color > 215) {
                light.color = 215;
            }
        }

        if(light.intensity > 0) {
            lightView->addLightSource(dest + (creature->m_walkOffset + (Point(SPRITE_SIZE / 1.8))) * scaleFactor, light);
        }
    }
}

void CreaturePainter::internalDrawOutfit(const CreaturePtr& creature, Point dest, float scaleFactor, bool useBlank, Otc::Direction_t direction)
{
    if(creature->m_outfitColor != Color::white)
        g_painter->setColor(creature->m_outfitColor);

    // outfit is a real creature
    if(creature->m_outfit.getCategory() == ThingCategoryCreature) {
        // xPattern => creature direction
        Otc::Direction_t xPattern;
        if(direction == Otc::NorthEast || direction == Otc::SouthEast)
            xPattern = Otc::East;
        else if(direction == Otc::NorthWest || direction == Otc::SouthWest)
            xPattern = Otc::West;
        else
            xPattern = direction;

				const int animationPhase = creature->getCurrentAnimationPhase();

        int zPattern = 0;
        if(creature->m_outfit.hasMount()) {
            const auto& datType = creature->rawGetMountThingType();

            dest -= datType->getDisplacement() * scaleFactor;
            ThingPainter::draw(datType, dest, scaleFactor, 0, xPattern, 0, 0, creature->getCurrentAnimationPhase(true), useBlank);

						if (!useBlank && creature->getLayers() > 1) {
							Color oldColor = g_painter->getColor();

							const Painter::CompositionMode oldComposition = g_painter->getCompositionMode();
							g_painter->setCompositionMode(Painter::CompositionMode_Multiply);

							Outfit::Clothes& mountClothes = creature->m_outfit.getMountClothes();
							const auto& colors = { mountClothes.getHeadColor(), mountClothes.getBodyColor(), mountClothes.getLegsColor(), mountClothes.getFeetColor() };

							for (const auto& color : colors) {
								g_painter->setColor(color.first);
								ThingPainter::draw(datType, dest, scaleFactor, color.second, xPattern, 0, zPattern, animationPhase, false);
							}

							g_painter->setColor(oldColor);
							g_painter->setCompositionMode(oldComposition);
						}

						dest += creature->getDisplacement() * scaleFactor;
						zPattern = std::min<int>(1, creature->getNumPatternZ() - 1);
        }

        const PointF jumpOffset = creature->m_jumpOffset * scaleFactor;
        dest -= Point(stdext::round(jumpOffset.x), stdext::round(jumpOffset.y));

        // yPattern => creature addon
        for(int yPattern = 0; yPattern < creature->getNumPatternY(); ++yPattern) {
            // continue if we dont have this addon
            if(yPattern > 0 && !(creature->m_outfit.getAddons() & (1 << (yPattern - 1))))
                continue;

            auto* datType = creature->rawGetThingType();
            ThingPainter::draw(datType, dest, scaleFactor, 0, xPattern, yPattern, zPattern, animationPhase, useBlank);

            if(!useBlank && creature->getLayers() > 1) {
                Color oldColor = g_painter->getColor();

                const Painter::CompositionMode oldComposition = g_painter->getCompositionMode();
                g_painter->setCompositionMode(Painter::CompositionMode_Multiply);

                const auto& colors = { creature->m_outfit.getClothes().getHeadColor(), creature->m_outfit.getClothes().getBodyColor(),
                    creature->m_outfit.getClothes().getLegsColor(),creature->m_outfit.getClothes().getFeetColor() };

                for(const auto& color : colors) {
                    g_painter->setColor(color.first);
                    ThingPainter::draw(datType, dest, scaleFactor, color.second, xPattern, yPattern, zPattern, animationPhase, false);
                }

                g_painter->setColor(oldColor);
                g_painter->setCompositionMode(oldComposition);
            }
        }
        // outfit is a creature imitating an item or the invisible effect
    } else {
        ThingType* type = g_things.rawGetThingType(creature->m_outfit.getAuxId(), creature->m_outfit.getCategory());

        int animationPhase = 0;
        int animationPhases = type->getAnimationPhases();
        int animateTicks = ITEM_TICKS_PER_FRAME;

        // when creature is an effect we cant render the first and last animation phase,
        // instead we should loop in the phases between
        if(creature->m_outfit.getCategory() == ThingCategoryEffect) {
            animationPhases = std::max<int>(1, animationPhases - 2);
            animateTicks = INVISIBLE_TICKS_PER_FRAME;
        }

        if(animationPhases > 1) {
            animationPhase = (g_clock.millis() % (animateTicks * animationPhases)) / animateTicks;
        }

        if(creature->m_outfit.getCategory() == ThingCategoryEffect)
            animationPhase = std::min<int>(animationPhase + 1, animationPhases);

        ThingPainter::draw(type, dest - (creature->getDisplacement() * scaleFactor), scaleFactor, 0, 0, 0, 0, animationPhase, useBlank);
    }

    if(creature->m_outfitColor != Color::white)
        g_painter->resetColor();
}

void CreaturePainter::drawOutfit(const CreaturePtr& creature, const Rect& destRect, bool resize)
{
    int frameSize;
    if(!resize)
        frameSize = creature->m_drawCache.frameSizeNotResized;
    else if((frameSize = creature->m_drawCache.exactSize) == 0)
        return;

    if(g_graphics.canUseFBO()) {
        const FrameBufferPtr& outfitBuffer = g_framebuffers.getTemporaryFrameBuffer();
        outfitBuffer->resize(Size(frameSize, frameSize));
        outfitBuffer->bind();
        internalDrawOutfit(creature, Point(frameSize - SPRITE_SIZE) + creature->getDisplacement(), 1, false, Otc::South);
        outfitBuffer->release();
        outfitBuffer->draw(destRect, Rect(0, 0, frameSize, frameSize));
    } else {
        const float scaleFactor = destRect.width() / static_cast<float>(frameSize);
        const Point dest = destRect.bottomRight() - (Point(SPRITE_SIZE) - creature->getDisplacement()) * scaleFactor;
        internalDrawOutfit(creature, dest, scaleFactor, false, Otc::South);
    }
}

void CreaturePainter::drawInformation(const CreaturePtr& creature, const Rect& parentRect, const Point& dest, float scaleFactor,
                                      const Point& drawOffset, const float horizontalStretchFactor, const float verticalStretchFactor, int drawFlags)
{
    if(creature->isDead()) // creature is dead
        return;

    const auto& tile = creature->getTile();
    if(!tile) return;

    if(!creature->canBeSeen())
        return;

    const PointF jumpOffset = creature->getJumpOffset() * scaleFactor;
    const auto creatureOffset = Point(16 - creature->getDisplacementX(), -creature->getDisplacementY() - 2);
    Position pos = creature->getPosition();
    Point p = dest - drawOffset;
    p += (creature->getDrawOffset() + creatureOffset) * scaleFactor - Point(stdext::round(jumpOffset.x), stdext::round(jumpOffset.y));
    p.x *= horizontalStretchFactor;
    p.y *= verticalStretchFactor;
    p += parentRect.topLeft();

    const bool useGray = tile->isCovered();
    auto fillColor = Color(96, 96, 96);

    if(!useGray) {
        if(g_game.getFeature(Otc::GameBlueNpcNameColor) && creature->isNpc() && creature->isFullHealth())
            fillColor = Color(0x66, 0xcc, 0xff);
        else fillColor = creature->m_informationColor;
    }

    // calculate main rects
    auto backgroundRect = Rect(p.x - (13.5), p.y, 27, 4);
    backgroundRect.bind(parentRect);

    const Size nameSize = creature->m_nameCache.getTextSize();
    auto textRect = Rect(p.x - nameSize.width() / 2.0, p.y - 12, nameSize);
    textRect.bind(parentRect);

    // distance them
    uint32 offset = 12;
    if(creature->isLocalPlayer()) {
        offset *= 2;
    }

    if(textRect.top() == parentRect.top())
        backgroundRect.moveTop(textRect.top() + offset);
    if(backgroundRect.bottom() == parentRect.bottom())
        textRect.moveTop(backgroundRect.top() - offset);

    // health rect is based on background rect, so no worries
    Rect healthRect = backgroundRect.expanded(-1);
    healthRect.setWidth((creature->m_healthPercent / 100.0) * 25);

    if(drawFlags & Otc::DrawBars) {
        g_painter->setColor(Color::black);
        g_painter->drawFilledRect(backgroundRect);

        g_painter->setColor(fillColor);
        g_painter->drawFilledRect(healthRect);

        if(drawFlags & Otc::DrawManaBar && creature->isLocalPlayer()) {
            LocalPlayerPtr player = g_game.getLocalPlayer();
            if(player) {
                backgroundRect.moveTop(backgroundRect.bottom());

                g_painter->setColor(Color::black);
                g_painter->drawFilledRect(backgroundRect);

                Rect manaRect = backgroundRect.expanded(-1);
                const double maxMana = player->getMaxMana();
                if(maxMana == 0) {
                    manaRect.setWidth(25);
                } else {
                    manaRect.setWidth(player->getMana() / (maxMana * 1.0) * 25);
                }

                g_painter->setColor(Color::blue);
                g_painter->drawFilledRect(manaRect);
            }
        }
    }

    if(drawFlags & Otc::DrawNames) {
        g_painter->setColor(fillColor);
        creature->m_nameCache.draw(textRect);
    }

    if(creature->m_skull != Otc::SkullNone && creature->m_skullTexture) {
        g_painter->resetColor();
        const auto skullRect = Rect(backgroundRect.x() + 13.5 + 12, backgroundRect.y() + 5, creature->m_skullTexture->getSize());
        g_painter->drawTexturedRect(skullRect, creature->m_skullTexture);
    }
    if(creature->m_shield != Otc::ShieldNone && creature->m_shieldTexture && creature->m_showShieldTexture) {
        g_painter->resetColor();
        const auto shieldRect = Rect(backgroundRect.x() + 13.5, backgroundRect.y() + 5, creature->m_shieldTexture->getSize());
        g_painter->drawTexturedRect(shieldRect, creature->m_shieldTexture);
    }
    if(creature->m_emblem != Otc::EmblemNone && creature->m_emblemTexture) {
        g_painter->resetColor();
        const auto emblemRect = Rect(backgroundRect.x() + 13.5 + 12, backgroundRect.y() + 16, creature->m_emblemTexture->getSize());
        g_painter->drawTexturedRect(emblemRect, creature->m_emblemTexture);
    }
    if(creature->m_type != Proto::CREATURE_TYPE_UNKNOW && creature->m_typeTexture) {
        g_painter->resetColor();
        const auto typeRect = Rect(backgroundRect.x() + 13.5 + 12 + 12, backgroundRect.y() + 16, creature->m_typeTexture->getSize());
        g_painter->drawTexturedRect(typeRect, creature->m_typeTexture);
    }
    if(creature->m_icon != Otc::NpcIconNone && creature->m_iconTexture) {
        g_painter->resetColor();
        const auto iconRect = Rect(backgroundRect.x() + 13.5 + 12, backgroundRect.y() + 5, creature->m_iconTexture->getSize());
        g_painter->drawTexturedRect(iconRect, creature->m_iconTexture);
    }
}

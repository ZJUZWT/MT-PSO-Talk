import "./index.css";
import { Composition } from "remotion";
import { MyComposition } from "./Composition";
import {REMOTION_PLAYER_CONFIG} from "./embed";

export const RemotionRoot: React.FC = () => {
  return (
    <>
      <Composition
        id="PsoConceptEvolution"
        component={MyComposition}
        durationInFrames={REMOTION_PLAYER_CONFIG.durationInFrames}
        fps={REMOTION_PLAYER_CONFIG.fps}
        width={REMOTION_PLAYER_CONFIG.compositionWidth}
        height={REMOTION_PLAYER_CONFIG.compositionHeight}
      />
    </>
  );
};
